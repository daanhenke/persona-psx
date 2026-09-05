"""Per-function matching harness.

Assembles the original function on its own, compiles a C candidate with the
Psy-Q cc1, and hands both objects to objdiff. objdiff understands relocations
and is what decomp-permuter scores against, so its percentage is the verdict -
no hand-rolled linking or byte comparison in the loop.

    tools/mfunc.py <target> <symbol>                 # show the target function
    tools/mfunc.py <target> <symbol> cand.c          # score a C candidate
    tools/mfunc.py <target> <symbol> cand.c --diff   # ...and show the diff
    tools/mfunc.py <target> <symbol> cand.c --diff --all   # include matching rows

Artifacts stay in build/match/<target>/<symbol>/ so objdiff can be re-run by hand.
"""
import bisect
import json
import os
import re
import shutil
import subprocess
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
GAME = "p1-jp"
VENV = os.path.join(ROOT, ".venv/bin/python3")
CC1 = os.path.join(ROOT, "bin/cc1-psx-26/cc1-psx-26")
MASPSX = os.path.join(ROOT, "tools/maspsx/maspsx.py")
OBJDIFF = os.path.join(ROOT, "bin/objdiff-cli")
CROSS = "mipsel-linux-gnu-"
AS = CROSS + "as"
NM = CROSS + "nm"
GCC = CROSS + "gcc"
READELF = CROSS + "readelf"

CC1FLAGS = ("-quiet -O2 -G0 -mcpu=3000 -msoft-float -mgas -gcoff -fgnu-linker "
            "-funsigned-char -fpeephole -ffunction-cse -fpcc-struct-return "
            "-fcommon -fverbose-asm -w").split()
ASFLAGS = ["-EL", "-march=r3000", "-mtune=r3000", "-no-pad-sections",
           "-I", os.path.join(ROOT, "include")]

LINE = re.compile(r"^\s*/\* ([0-9A-Fa-f]+) ([0-9A-Fa-f]{8}) ([0-9A-Fa-f]{8}) \*/")

FUNC_HDR = ('.include "macro.inc"\n'
            '.set noat\n'
            '.set noreorder\n'
            '.section .text, "ax"\n')

# When a %hi/%lo pair lands inside a large data run, spimdisasm renders it as
# "D_8009FB00 + 0x28" because it has no symbol for that exact address, while the
# C source declares D_8009FB28. Fold the offset into the name so the two objects
# agree - objdiff and decomp-permuter both compare symbols. Only data symbols
# are folded; func_/jtbl_ offsets are real intra-function references.
OFFREF = re.compile(r"D_([0-9A-Fa-f]{8})\s*\+\s*(0x[0-9A-Fa-f]+)")


def run(cmd, **kw):
    r = subprocess.run(cmd, capture_output=True, text=True, **kw)
    if r.returncode != 0:
        sys.exit("command failed: %s\n%s\n%s"
                 % (" ".join(map(str, cmd)), r.stdout, r.stderr))
    return r


def split_asm_path(target):
    return os.path.join(ROOT, "asm", GAME, target, target + ".s")


def load_target(target, symbol):
    """Original bytes + asm lines for `symbol`, from splat's output."""
    words = []
    lines = []
    vram = None
    inside = False
    for line in open(split_asm_path(target)):
        s = line.strip()
        if s.startswith("glabel ") or s.startswith("dlabel "):
            if inside:
                break
            inside = s.split()[1] == symbol
            continue
        if s.startswith("endlabel ") and inside:
            break
        if not inside or s.startswith("nonmatching"):
            continue
        lines.append(line.rstrip())
        m = LINE.match(line)
        if m:
            if vram is None:
                vram = int(m.group(2), 16)
            words.append(bytes.fromhex(m.group(3)))
    if not words:
        sys.exit("symbol %r not found in %s" % (symbol, split_asm_path(target)))
    return vram, b"".join(words), lines


NAMED = re.compile(r"\b(?:func|D)_([0-9A-F]{8})\b")
PROVIDE = re.compile(r"PROVIDE\(([A-Za-z_]\w*) = 0x([0-9A-Fa-f]{8})\);")
# %hi(D_7FFFFF) / %lo(D_7FFFFF) - a constant spimdisasm mistook for an address.
CONSTREF = re.compile(r"%(hi|lo)\(\s*D_([0-9A-Fa-f]+)\s*\)")
# The $at expansion of an indexed load, in its two forms. Which one the
# assembler picked says whether the source named a symbol or wrote the address
# out as a literal - see fold_literal_at_refs.
ATIDX = re.compile(r"\baddu\s+\$at,\s*\$(?!at\b)(\w+),\s*\$at\b")
HIREF = re.compile(r"%hi\(\s*([A-Za-z_]\w*)\s*\)")
LOREF = re.compile(r"%lo\(\s*([A-Za-z_]\w*)\s*\)\(\$at\)")
# splat writes an address it did NOT symbolise in this explicit form, so a
# function that builds one is proof that region is reached by literal.
RAWADDR = re.compile(r"\(\s*0x([0-9A-Fa-f]{8})\s*>>\s*16\s*\)")
ANYHI = re.compile(r"%hi\(\s*([A-Za-z_]\w*)\s*\)")
ANYLO = re.compile(r"%lo\(\s*([A-Za-z_]\w*)\s*\)\((\$\w+)\)")


def load_names(target):
    """address -> real name, from the generated symbol map."""
    path = os.path.join(ROOT, "config", GAME, "%s.names.ld" % target)
    out = {}
    if os.path.exists(path):
        for line in open(path):
            m = PROVIDE.search(line)
            if m:
                out.setdefault(int(m.group(2), 16), m.group(1))
    return out


def gp_base(target):
    """Runtime $gp for a target, if known (see config/<game>/gp.txt)."""
    path = os.path.join(ROOT, "config", GAME, "gp.txt")
    if not os.path.exists(path):
        return None
    for line in open(path):
        line = line.split("//")[0].split()
        if len(line) == 2 and line[0] == target:
            return int(line[1], 16)
    return None


GPREF = re.compile(r"(-?(?:0x)?[0-9A-Fa-f]+)\(\$gp\)")


def _hi(value):
    return "0x%X" % (((value + 0x8000) >> 16) & 0xFFFF)


def _lo(value):
    low = value & 0xFFFF
    return "%d" % (low - 0x10000 if low >= 0x8000 else low)


def fold_literal_at_refs(lines, names=None):
    """Undo spimdisasm's invented symbol where the source wrote an address out.

    An indexed load reaches memory through $at, and the assembler expands it two
    different ways:

        lbu $2,sym($2)     -> lui $at,%hi(sym) / addu $at,$at,$2  / lbu ...
        lbu $2,0x801F29C8($2) -> lui $at,%hi   / addu $at,$2,$at  / lbu ...

    The operand order of that `addu` is therefore a byte-level record of which
    one the original source used. spimdisasm renders both as %hi/%lo of a
    symbol it invented, so a candidate that correctly writes the literal scores
    below 100% forever. Fold the literal ones back.

    This is not cosmetic: the game reaches its 0x801Dxxxx-0x801Fxxxx work area
    by hardcoded address (main bzeros 0x801F0000 without naming anything in
    it), and that is thousands of instructions across the disc.

    Keyed on the address, not on the symbol still carrying a default D_ name -
    the `addu` operand order says the source used a literal no matter what we
    have since called that address, so giving it a name must not silently break
    every function that reaches it.
    """
    addr_of = {n: a for a, n in (names or {}).items()}
    out = list(lines)
    for i, ln in enumerate(out):
        if not ATIDX.search(ln):
            continue
        # The lui and the load sit either side of the addu, but scheduling can
        # put an unrelated instruction between them.
        hi_at = sym = None
        for j in range(i - 1, max(i - 5, -1), -1):
            m = HIREF.search(out[j])
            if m and "lui" in out[j]:
                hi_at, sym = j, m.group(1)
                break
        if sym is None:
            continue
        lo_at = None
        for k in range(i + 1, min(i + 5, len(out))):
            m = LOREF.search(out[k])
            if m and m.group(1) == sym:
                lo_at = k
                break
        if lo_at is None:
            continue
        if sym.startswith("D_") and len(sym) == 10:
            value = int(sym[2:], 16)
        elif sym in addr_of:
            value = addr_of[sym]
        else:
            continue
        out[hi_at] = HIREF.sub(_hi(value), out[hi_at], count=1)
        out[lo_at] = LOREF.sub("%s($at)" % _lo(value), out[lo_at], count=1)

    # Second pass. spimdisasm symbolises some `lui/lo` pairs and leaves others
    # as raw addresses, so one function can reach the same object both ways -
    # the play-time counter builds 0x801F29BC with lui/ori and then loads
    # +1/+2/+3 through invented symbols. A raw address in the function is proof
    # that its neighbourhood is literal here, so fold the invented ones to
    # match; otherwise a correct candidate can never agree on the relocation.
    literals = set()
    for ln in out:
        for m in RAWADDR.finditer(ln):
            literals.add(int(m.group(1), 16))
    if not literals:
        return out

    def addr_of_sym(name):
        if name.startswith("D_") and len(name) == 10:
            try:
                return int(name[2:], 16)
            except ValueError:
                return None
        return addr_of.get(name)

    def near_literal(a):
        return a is not None and any(abs(a - lit) <= 0x400 for lit in literals)

    for i, ln in enumerate(out):
        m = ANYHI.search(ln)
        if m and near_literal(addr_of_sym(m.group(1))):
            out[i] = ANYHI.sub(_hi(addr_of_sym(m.group(1))), ln, count=1)
            continue
        m = ANYLO.search(ln)
        if m and near_literal(addr_of_sym(m.group(1))):
            a = addr_of_sym(m.group(1))
            out[i] = ANYLO.sub("%s(%s)" % (_lo(a), m.group(2)), ln, count=1)
    return out


def normalize_asm(lines, names=None, gp=None):
    """Make the target asm agree with the C on symbol names.

    Three mismatches otherwise show up as permanent objdiff penalties even when
    the code is identical:
      - spimdisasm's "D_8009FB00 + 0x28" offset form vs the C's D_8009FB28
      - address-derived names where the C uses the real one
        (func_80012B2C vs CdSearchFileLoc)
      - a lui/addiu pair building a large *constant*, which spimdisasm
        mis-reads as an address and invents a symbol for (0x800000-1 becomes
        %hi(D_7FFFFF)/%lo(D_7FFFFF)). Anything below the 0x80000000 RAM base
        cannot be an address, so fold those back to literals.
    """
    def fold(m):
        return "D_%08X" % (int(m.group(1), 16) + int(m.group(2), 16))

    # Sorted named addresses, for rebasing references into the middle of a
    # named object.
    based = sorted(names) if names else []

    def rename(m):
        addr = int(m.group(1), 16)
        hit = (names or {}).get(addr)
        if hit:
            return hit
        # A reference into the middle of a named array or struct: spimdisasm
        # invents its own symbol (D_800622FC) where the C says g_cd_queue+0xc.
        # Rebase onto the nearest named symbol below it.
        if based and m.group(0).startswith("D_"):
            i = bisect.bisect_right(based, addr) - 1
            below = addr - based[i] if i >= 0 else None
            j = i + 1
            above = based[j] - addr if j < len(based) else None
            # A reference can legitimately sit just *below* a named object:
            # gcc reaches arr[i-1] by building &arr[-1] and indexing that, so
            # the base it materialises is one element before the array.
            # spimdisasm has no symbol there and blames whatever precedes it,
            # which is usually an unrelated object.
            #
            # Only take that reading when the address is clearly not a member
            # of the object below - deep inside it, but flush against the one
            # above, and aligned as an array base would be. Without the
            # below-distance guard this misreads an ordinary byte reference
            # into a string that happens to sit just before another symbol.
            if (above is not None and above <= 0x10 and addr % 4 == 0
                    and (below is None or below > 0x10)):
                return "%s-0x%X" % (names[based[j]], above)
            if below is not None and 0 < below <= 0x400:
                return "%s+0x%X" % (names[based[i]], below)
        return m.group(0)

    def unconst(m):
        value = int(m.group(2), 16)
        if value >= 0x80000000:
            return m.group(0)
        if m.group(1) == "hi":
            return "0x%X" % (((value + 0x8000) >> 16) & 0xFFFF)
        low = value & 0xFFFF
        return "%d" % (low - 0x10000 if low >= 0x8000 else low)

    def gprel(m):
        raw = m.group(1)
        off = int(raw, 16) if raw.lower().startswith(("0x", "-0x")) else int(raw, 0)
        sym = (names or {}).get(gp + off)
        return "%%gp_rel(%s)($gp)" % sym if sym else m.group(0)

    out = []
    # Runs first: once an invented symbol is folded to a literal, `rename` must
    # not turn it back into a name the candidate cannot use.
    for ln in fold_literal_at_refs(lines, names):
        ln = OFFREF.sub(fold, ln)
        ln = CONSTREF.sub(unconst, ln)
        if names:
            ln = NAMED.sub(rename, ln)
        # splat disassembles the *linked* binary, so a small-data access shows
        # up as a bare "0($gp)". A compiled candidate emits a %gp_rel
        # relocation instead; resolve the target's form once $gp is known.
        if gp is not None and names:
            ln = GPREF.sub(gprel, ln)
        out.append(ln)
    return out


FLAGS_DIRECTIVE = re.compile(r"/\*\s*cc1flags:\s*(.*?)\s*\*/")


def cc1flags_for(cfile):
    """Per-file compiler flags.

    Not every translation unit was built the same way - parts of the sub-EXEs
    use -O0 and a non-zero -G (frame pointers and $gp-relative accesses give it
    away), so a source can override the defaults with:

        /* cc1flags: -O0 -G8 */
    """
    text = open(cfile, encoding="utf-8", errors="replace").read(4096)
    m = FLAGS_DIRECTIVE.search(text)
    if not m:
        return CC1FLAGS
    override = m.group(1).split()
    base = [f for f in CC1FLAGS
            if not any(f.startswith(o.split("=")[0][:2]) for o in override
                       if o.startswith("-O") or o.startswith("-G"))]
    return base + override


def compile_c(cfile, outdir):
    """cc1 is the compiler proper, so cpp must run first. maspsx reads stdin."""
    i_path = os.path.join(outdir, "cand.i")
    s_path = os.path.join(outdir, "cand.s")
    o_path = os.path.join(outdir, "cand.o")
    run([GCC, "-E", "-nostdinc", "-undef", "-D__GNUC__=2",
         "-I", os.path.join(ROOT, "include"),
         "-I", os.path.join(ROOT, "include", "psyq"),
         "-I", os.path.dirname(cfile),
         cfile, "-o", i_path])
    run([CC1] + cc1flags_for(cfile) + [i_path, "-o", s_path])
    piped = subprocess.run(
        [VENV, MASPSX, "--expand-div", "--aspsx-version=2.34"],
        input=open(s_path).read(), capture_output=True, text=True)
    if piped.returncode != 0:
        sys.exit("maspsx failed:\n" + piped.stderr)
    run([AS] + ASFLAGS + ["-o", o_path, "-"], input=piped.stdout)
    return o_path


def c_symbol(o_path, prefer=None):
    """Global text symbol defined by the candidate.

    `prefer` lets a source hold several functions: pass the real name of the one
    being matched and it wins over whichever happens to come first.
    """
    found = []
    for line in run([NM, "--defined-only", o_path]).stdout.splitlines():
        p = line.split()
        if len(p) == 3 and p[1] == "T":
            found.append(p[2])
    if prefer and prefer in found:
        return prefer
    return found[0] if found else None


# A switch compiles to a bounds test, an indexed load out of .rodata and a jump
# through the loaded register.
JRREG = re.compile(r"\bjr\s+\$(?!ra\b)(\w+)\b")
SWLOAD = re.compile(r"\blw\s+\$(\w+),\s*%lo\(\s*([^)]+?)\s*\)\(\$at\)")
SWHI = re.compile(r"\blui\s+\$at,\s*%hi\(\s*([^)]+?)\s*\)")
SWBOUND = re.compile(r"\bsltiu?\s+\$\w+,\s*\$\w+,\s*(0x[0-9A-Fa-f]+|\d+)\b")
SYMADDR = re.compile(r"^(?:D_([0-9A-Fa-f]{8})|(\w+))\s*(?:\+\s*(0x[0-9A-Fa-f]+))?$")


def name_addrs(target):
    """real name -> address, the other direction from load_names.

    A jump table that falls inside an object we have already named is written
    `thatname + 0xNN`, so resolving the dispatch needs the map this way round.
    """
    return {name: addr for addr, name in load_names(target).items()}


def data_words(target, addr, count):
    """`count` words starting at `addr`, read out of splat's data lines."""
    out = {}
    want = set(range(addr, addr + count * 4, 4))
    for line in open(split_asm_path(target)):
        m = LINE.match(line)
        if not m or ".word" not in line:
            continue
        vram = int(m.group(2), 16)
        if vram in want:
            raw = m.group(3)
            out[vram] = int.from_bytes(bytes.fromhex(raw), "little")
            if len(out) == count:
                break
    return [out.get(addr + i * 4) for i in range(count)]


def text_span(o_path, sym):
    """(offset, size) of one function inside its object's .text."""
    r = subprocess.run([READELF, "-sW", o_path], capture_output=True, text=True)
    for line in r.stdout.splitlines():
        f = line.split()
        if len(f) >= 8 and f[3] == "FUNC" and f[7] == sym:
            return int(f[1], 16), int(f[2])
    return None


def rebuild_jump_tables(lines, target, cand_o=None, sym=None):
    """Give the target's switch tables back to the function that owns them.

    splat leaves a jump table sitting in the overlay's data run, so the dispatch
    reads it through whatever symbol happens to precede it. A compiled candidate
    emits its own table into .rodata, and the two relocations can never agree.
    Re-emitting the table as a local .rodata label here puts both sides on the
    same footing: the case addresses still come from the original, so a
    candidate whose cases land in a different order still fails.
    """
    if target is None:
        return lines, []

    body = list(lines)
    rodata = []
    by_name = name_addrs(target)
    vram_at = {}
    for i, ln in enumerate(body):
        m = LINE.match(ln)
        if m:
            vram_at[int(m.group(2), 16)] = i
    if not vram_at:
        return body, []
    lo_vram, hi_vram = min(vram_at), max(vram_at)

    inserts = {}
    tables = []
    for i, ln in enumerate(body):
        m = JRREG.search(ln)
        if not m:
            continue
        reg = m.group(1)
        load = table = None
        for j in range(i - 1, max(i - 8, -1), -1):
            mm = SWLOAD.search(body[j])
            if mm and mm.group(1) == reg:
                load, table = j, mm.group(2)
                break
        if load is None:
            continue
        hi = None
        for j in range(load - 1, max(load - 8, -1), -1):
            mm = SWHI.search(body[j])
            if mm and mm.group(1) == table:
                hi = j
                break
        if hi is None:
            continue
        ma = SYMADDR.match(table)
        if not ma:
            continue
        if ma.group(1):
            base = int(ma.group(1), 16)
        else:
            base = by_name.get(ma.group(2))
        if base is None:
            continue
        addr = base + (int(ma.group(3), 16) if ma.group(3) else 0)
        # The bounds test is the nearest compare above the dispatch, but an
        # unrelated one can be sitting in a delay slot between them, so try
        # every candidate and keep the first whose table reads back as case
        # addresses inside this function.
        entries = None
        for j in range(hi - 1, max(hi - 12, -1), -1):
            mm = SWBOUND.search(body[j])
            if not mm:
                continue
            count = int(mm.group(1), 0)
            if not count or count > 256:
                continue
            got = data_words(target, addr, count)
            if all(e is not None and lo_vram <= e <= hi_vram for e in got):
                entries = got
                break
        if entries is None:
            continue

        k = len(tables)
        for e in entries:
            label = "$Ljt%dc%X" % (k, e)
            at = vram_at[e]
            if label + ":" not in inserts.setdefault(at, []):
                inserts[at].append(label + ":")
        tables.append((k, entries))
        body[hi] = SWHI.sub("lui $at, %%hi($Ljt%d)" % k, body[hi], count=1)
        body[load] = SWLOAD.sub(
            "lw $%s, %%lo($Ljt%d)($at)" % (reg, k), body[load], count=1)

    if not tables:
        return body, []

    out = []
    for i, ln in enumerate(body):
        for lbl in inserts.get(i, []):
            out.append("  " + lbl)
        out.append(ln)

    # A candidate holding several switches puts this function's table wherever
    # the ones before it left off, and the dispatch relocation carries that
    # offset. Find the run whose case spacing matches and pad to sit at the
    # same place; a table that is not in the candidate at all gets no padding,
    # and the relocation then disagrees, which is the answer we want.
    rodata = ['.section .rodata', '.align 2']
    placed = 0
    theirs = rodata_runs(cand_o) if cand_o else []
    # Two switches of the same shape in one file - the same bar drawn from two
    # tables, say - are told apart by which function their cases point into.
    span = text_span(cand_o, sym) if cand_o and sym else None
    if span:
        lo, size = span
        theirs = [(off, cases) for off, cases in theirs
                  if all(lo <= c < lo + size for c in cases)]
    for k, entries in tables:
        shape = [e - entries[0] for e in entries]
        for i, (off, cases) in enumerate(theirs):
            if len(cases) == len(entries) and \
                    [c - cases[0] for c in cases] == shape and off >= placed:
                if off > placed:
                    rodata.append("    .space %d" % (off - placed))
                    placed = off
                theirs.pop(i)
                break
        rodata.append("$Ljt%d:" % k)
        for e in entries:
            rodata.append("    .word $Ljt%dc%X" % (k, e))
        placed += 4 * len(entries)
    return out, rodata


HEXDUMP = re.compile(r"^\s*0x[0-9a-f]+((?:\s+[0-9a-f]{2,8})+)\s")
RELROW = re.compile(
    r"^([0-9a-f]{8})\s+[0-9a-f]{8}\s+\S+\s+[0-9a-f]{8}\s+(\S+)")


def rodata_image(o_path):
    """(.rodata words, {offset: relocated symbol}) for one object.

    o32 relocations carry no addend, so a jump table's case addresses live in
    the section words themselves.
    """
    data, rels, section = [], {}, None
    r = subprocess.run([READELF, "-x", ".rodata", "-r", o_path],
                       capture_output=True, text=True)
    for line in r.stdout.splitlines():
        if line.startswith("Hex dump of section"):
            section = "hex"
            continue
        if line.startswith("Relocation section"):
            section = "rel" if ".rel.rodata" in line else None
            continue
        if section == "hex":
            m = HEXDUMP.match(line)
            if m:
                data.append(m.group(1).replace(" ", ""))
        elif section == "rel":
            m = RELROW.match(line)
            if m:
                rels[int(m.group(1), 16)] = m.group(2)
    raw = bytes.fromhex("".join(data))
    words = [int.from_bytes(raw[i:i + 4], "little")
             for i in range(0, len(raw) - 3, 4)]
    return words, rels


def rodata_runs(o_path):
    """The object's switch tables, as (byte offset, [case .text offsets]).

    A table is a run of consecutive words each relocated against .text, which
    is what a jump table looks like and what a string or float literal does
    not.
    """
    words, rels = rodata_image(o_path)
    runs, cur, start = [], [], 0
    for i, w in enumerate(words):
        if rels.get(i * 4) == ".text":
            if not cur:
                start = i * 4
            cur.append(w)
            continue
        if cur:
            runs.append((start, cur))
            cur = []
    if cur:
        runs.append((start, cur))
    return runs


def tables_agree(target_o, cand_o):
    """Do both objects dispatch through the same switch table?

    Only meaningful when the target has one - build_target_obj puts nothing but
    tables in .rodata, so an empty section means there was no switch to check.
    The two objects place the function at different offsets in .text, so the
    case addresses differ by a constant; what has to agree is their order and
    spacing.
    """
    want = rodata_runs(target_o)
    if not want:
        return True
    got = {off: cases for off, cases in rodata_runs(cand_o)}
    for off, cases in want:
        theirs = got.get(off)
        if theirs is None or len(theirs) != len(cases):
            return False
        bias = theirs[0] - cases[0]
        if any(b - a != bias for a, b in zip(cases, theirs)):
            return False
    return True


def build_target_obj(lines, sym, outdir, names=None, gp=None, target=None,
                     cand_o=None):
    """Assemble the original function alone, under the candidate's symbol name."""
    s_path = os.path.join(outdir, "target.s")
    o_path = os.path.join(outdir, "target.o")
    body, rodata = rebuild_jump_tables(lines, target, cand_o, sym)
    with open(s_path, "w") as f:
        f.write(FUNC_HDR)
        f.write(".global %s\n%s:\n" % (sym, sym))
        f.write("\n".join(normalize_asm(body, names, gp)) + "\n")
        if rodata:
            f.write("\n".join(rodata) + "\n")
    run([AS] + ASFLAGS + ["-o", o_path, s_path])
    return o_path


def objdiff_json(target_o, cand_o, sym):
    r = subprocess.run(
        [OBJDIFF, "diff", "-1", target_o, "-2", cand_o,
         "--format", "json", "-o", "-", sym],
        capture_output=True, text=True, cwd=ROOT)
    if r.returncode != 0:
        print("  objdiff failed: " + (r.stderr or r.stdout).strip()[:400])
        return None
    try:
        return json.loads(r.stdout)
    except ValueError:
        print("  objdiff produced unparseable output")
        return None


def pick_symbol(side, sym):
    """objdiff puts per-instruction diffs on <side>.symbols[].instructions."""
    best = None
    for s in (side or {}).get("symbols", []):
        if s.get("kind") != "SYMBOL_FUNCTION":
            continue
        if s.get("name") == sym:
            return s
        if best is None and s.get("instructions"):
            best = s
    return best


def insn_text(ins):
    if not isinstance(ins, dict):
        return ""
    return (ins.get("instruction") or {}).get("formatted", "")


MARK = {"DIFF_NONE": " ", "DIFF_REPLACE": "~", "DIFF_DELETE": "<",
        "DIFF_INSERT": ">", "DIFF_OP_MISMATCH": "~", "DIFF_ARG_MISMATCH": "~"}

W = 44


def render(d, sym, show_all=False):
    left = pick_symbol(d.get("left"), sym)
    right = pick_symbol(d.get("right"), sym)
    if not left or not left.get("instructions"):
        print("  objdiff returned no instructions for %s" % sym)
        return
    li = left.get("instructions") or []
    ri = (right or {}).get("instructions") or []

    print("    %-6s %-*s %s" % ("", W, "target", "current"))
    print("    " + "-" * (W + 48))
    for i in range(max(len(li), len(ri))):
        a = li[i] if i < len(li) else None
        b = ri[i] if i < len(ri) else None
        kind = (a or b or {}).get("diff_kind", "") or ""
        mark = MARK.get(kind, " ")
        if mark == " " and not show_all:
            continue
        addr = (a or {}).get("instruction", {}).get("address", 0)
        try:
            addr = int(addr)
        except (TypeError, ValueError):
            addr = 0
        print("  %s %04X   %-*s %s" % (mark, addr, W,
                                       insn_text(a)[:W], insn_text(b)[:W]))


def main():
    if len(sys.argv) < 3:
        sys.exit(__doc__)
    target = sys.argv[1]
    symbol = sys.argv[2]
    vram, orig, lines = load_target(target, symbol)
    print("%s:%s  vram=0x%08X  %d bytes (%d instructions)"
          % (target, symbol, vram, len(orig), len(orig) // 4))
    if len(sys.argv) == 3 or sys.argv[3].startswith("--"):
        return 0

    cfile = os.path.abspath(sys.argv[3])
    # MFUNC_TAG keeps concurrent runs of the same target/symbol from sharing a
    # scratch directory, which is what tools/tryv.py needs to score several
    # candidate spellings of one function at once.
    outdir = os.path.join(ROOT, "build", "match", target,
                          symbol + os.environ.get("MFUNC_TAG", ""))
    shutil.rmtree(outdir, ignore_errors=True)
    os.makedirs(outdir)

    names = load_names(target)
    cand_o = compile_c(cfile, outdir)
    sym = c_symbol(cand_o, names.get(vram)) or symbol
    target_o = build_target_obj(lines, sym, outdir, names, gp_base(target),
                               target, cand_o)

    d = objdiff_json(target_o, cand_o, sym)
    if d is None:
        return 1
    left = pick_symbol(d.get("left"), sym)
    pct = (left or {}).get("match_percent")
    print()
    if pct is None:
        print("  RESULT : objdiff could not compare")
        return 1
    if pct >= 100.0 and not tables_agree(target_o, cand_o):
        print("  objdiff: %.2f%%   mismatch - the switch table differs" % pct)
        return 1
    if pct >= 100.0:
        print("  objdiff: %.2f%%   EXACT MATCH" % pct)
        return 0
    print("  objdiff: %.2f%%   mismatch" % pct)
    if "--diff" in sys.argv:
        print()
        render(d, sym, show_all="--all" in sys.argv)
    return 1


if __name__ == "__main__":
    sys.exit(main())
