"""Mark the Psy-Q SDK so it stops counting as work to do.

Every target ships the parts of the Psy-Q libraries it uses, linked in from
Sony's own .LIB files. That code is not ours to decompile - the invariant in
CLAUDE.md says so - and it will be linked from the real libraries in the end.
Left in the denominator it is simply noise: `open` is 123KB of which almost all
is libgs and libspu, so its progress figure says nothing about the game.

This writes config/<game>/sdk.txt, one row per SDK function, which
tools/progress.py subtracts and reports on a line of its own. The file is
checked in and meant to be read: if a row in it is wrong, that is a function
being written off that should have been decompiled, so the classification is
kept explicit rather than recomputed silently.

    tools/gen_sdk.py            # rewrite config/p1-jp/sdk.txt
    tools/gen_sdk.py --dry-run  # print the summary, change nothing

Three things decide it, in order:

1. The overlays contain no SDK at all. Ghidra's PsyQ signature analyser matches
   nothing in any of the seven overlay spaces - they call into the resident
   EXE for all of it - so every overlay function is game code by construction.
   Only main and the four sub-EXEs are classified here.

2. A name the signature analyser applied. Two forms: `<LIB>_OBJ_<offset>` for a
   function inside a library object it could not name individually, and the
   real API names, which we recognise by their declarations in include/psyq.
   EXTRA covers the runtime and libc pieces those headers do not declare.

3. An unnamed function *every one of whose callers is SDK*. The analyser misses
   small stubs sitting between the routines it did match - 23 of them in main,
   twelve to forty-four bytes each - and this picks them up without ever
   absorbing game code, because game code always has a game caller. Callbacks
   cannot leak the other way: the SDK reaches those through function pointers,
   and only direct `jal` edges are followed here.

Anything already in decomp.txt is game code by definition and is never marked,
which is the check that rules 2 and 3 have not overreached.
"""
import json
import os
import re
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
GAME = "p1-jp"
GHIDRA = os.path.join(ROOT, "config", GAME, "ghidra")
DECOMP = os.path.join(ROOT, "config", GAME, "decomp.txt")
PSYQ = os.path.join(ROOT, "include", "psyq")
ASM = os.path.join(ROOT, "asm", GAME)
OUT = os.path.join(ROOT, "config", GAME, "sdk.txt")

# Only these carry linked-in library code; see rule 1 above.
EXE = {
    "main": ("SLPS_005_00.json", "ram"),
    "atlus": ("ATLUS_EXE.json", "ram"),
    "open": ("OPEN_EXE.json", "ram"),
    "movie": ("MOVIE_EXE.json", "ram"),
    "end": ("END_EXE.json", "ram"),
}

# `PRNT_OBJ_594`, `2D_BG0_OBJ_1B4`, `VMANAGER_OBJ_2A94`: the analyser's name for
# a function inside a library object it matched but could not name.
OBJ = re.compile(r"^[A-Za-z0-9_]+_OBJ_[0-9A-Fa-f]+$")

# The C runtime, soft-float and start-up pieces. None of these are declared in
# include/psyq (they are not API), and all come out of Sony's libraries.
EXTRA = set("""
start stup0 stup1 _start __main __do_global_ctors __do_global_dtors
printf sprintf vsprintf puts putchar fprintf
memcpy memset memmove bcopy bzero bcmp
strcpy strncpy strcat strncat strcmp strncmp strlen strchr strrchr strstr
atoi atol abs labs rand srand qsort bsearch
malloc free calloc realloc InitHeap
__adddf3 __subdf3 __muldf3 __divdf3 __negdf2 __fixdfsi __floatsidf
__addsf3 __subsf3 __mulsf3 __divsf3 __negsf2 __fixsfsi __floatsisf
__extendsfdf2 __truncdfsf2 __cmpdf2 __cmpsf2 __eqdf2 __nedf2 __ltdf2
__ledf2 __gtdf2 __gedf2 __udivsi3 __umodsi3 __divsi3 __modsi3
_mainasu _comp_mant _add_mant_d _mul_mant_d _dbl_shift _dbl_shift_us
_err_math _cmp _reset
""".split())

GLABEL = re.compile(r"^glabel (\S+)")
JAL = re.compile(r"^\s*/\* \S+ \S+ \S+ \*/\s+jal\s+(\S+)")
ADDR = re.compile(r"^func_([0-9A-Fa-f]{8})$")
# `int GsInitGraph(...)`, `void CdInit(void)`, ... - the identifier before the
# open paren of a declaration. Deliberately loose: a false positive here can
# only matter if the binary also has a function of that name.
DECL = re.compile(r"\b([A-Za-z_][A-Za-z0-9_]*)\s*\(")
BLOCK_COMMENT = re.compile(r"/\*.*?\*/", re.S)
LINE_COMMENT = re.compile(r"//[^\n]*")
# Words the loose regex above will happily read as a function name.
NOT_A_NAME = {"if", "while", "for", "switch", "return", "sizeof", "defined",
              "struct", "union", "enum", "void", "char", "short", "int",
              "long", "unsigned", "signed", "const", "volatile", "static",
              "extern", "typedef", "register"}


def psyq_names():
    """Every function name declared by the vendored Psy-Q headers.

    Declarations here run over several lines and carry comments between the
    parameters, so this works on the whole file at once: strip the comments and
    the preprocessor, split on `;`, and take the first identifier before a `(`
    in each statement. Reading it line by line missed every declaration that
    documents its arguments - LoadTPage and a dozen others - and each one it
    missed showed up as an SDK function we had supposedly not accounted for.
    """
    out = set()
    for f in sorted(os.listdir(PSYQ)):
        if not f.endswith(".h"):
            continue
        text = open(os.path.join(PSYQ, f), errors="replace").read()
        text = BLOCK_COMMENT.sub(" ", text)
        text = LINE_COMMENT.sub(" ", text)
        text = "\n".join(l for l in text.split("\n")
                         if not l.lstrip().startswith("#"))
        for stmt in text.split(";"):
            s = " ".join(stmt.split())
            if not s or "(" not in s or "{" in s or "}" in s:
                continue
            if s.startswith(("typedef", "struct", "union", "enum")):
                continue
            for m in DECL.finditer(s):
                if m.group(1) not in NOT_A_NAME:
                    out.add(m.group(1))
                    break
    return out


def ghidra(target):
    """(name by address, size by address) for one target's own space."""
    jf, space = EXE[target]
    path = os.path.join(GHIDRA, jf)
    if not os.path.exists(path):
        return {}, {}
    sp = json.load(open(path))["spaces"][space]
    # Ghidra keeps the GTE macros in a synthetic space of their own at
    # 0x2000xxxx, one byte each. They are not code in this binary.
    blo, bhi = sp["blocks"][0]["start"], sp["blocks"][0]["end"]
    names = {int(a, 16): n for a, n in sp.get("names", {}).items()
             if blo <= int(a, 16) <= bhi}
    size = {}
    for ep, lo, hi in sp.get("funcs", []):
        if blo <= ep <= bhi:
            size[ep] = size.get(ep, 0) + (hi - lo + 1)
    return names, size


def edges(target):
    """symbol -> the symbols it calls with a direct jal."""
    path = os.path.join(ASM, target, target + ".s")
    out, cur = {}, None
    if not os.path.exists(path):
        return out
    for line in open(path):
        m = GLABEL.match(line)
        if m:
            cur = m.group(1)
            out[cur] = []
            continue
        if line.startswith("endlabel"):
            cur = None
            continue
        if cur is None:
            continue
        m = JAL.match(line)
        if m:
            out[cur].append(m.group(1))
    return out


def asm_order(target):
    """Every function in the split asm, by address.

    splat finds code Ghidra never turned into a function - five in main - so
    the Ghidra listing alone leaves holes in the middle of a library. This is
    what rule 4 walks.
    """
    path = os.path.join(ASM, target, target + ".s")
    out = []
    if not os.path.exists(path):
        return out
    for line in open(path):
        m = GLABEL.match(line)
        if m:
            a = ADDR.match(m.group(1))
            if a:
                out.append(int(a.group(1), 16))
    return sorted(out)


def decompiled():
    """(target, symbol) pairs we have already claimed as game code."""
    out = set()
    if not os.path.exists(DECOMP):
        return out
    for line in open(DECOMP):
        f = line.split("//")[0].split()
        if len(f) >= 2:
            out.add((f[0], f[1]))
    return out


def classify(target, api, claimed):
    """Returns {address: name} for the SDK functions of one EXE target."""
    names, size = ghidra(target)
    call = edges(target)

    def addr_of(sym):
        m = ADDR.match(sym)
        return int(m.group(1), 16) if m else None

    # Rule 2: the analyser named it, and it is not something we have claimed.
    sdk = set()
    for addr, name in names.items():
        if name.startswith(("FUN_", "thunk_FUN_")):
            continue
        if name == "main":
            continue                    # the game's entry point, not libc's
        if ("%s" % target, "func_%08X" % addr) in claimed:
            continue
        if OBJ.match(name) or name in EXTRA or name in api:
            sdk.add(addr)

    # Callers, over direct jal edges only.
    callers = {}
    for sym, callees in call.items():
        a = addr_of(sym)
        if a is None:
            continue
        for c in callees:
            ca = addr_of(c)
            if ca is not None:
                callers.setdefault(ca, set()).add(a)

    # Rule 3: an unnamed function all of whose callers are already SDK. Repeat
    # until it settles - a stub can call another stub.
    unnamed = {a for a, n in names.items()
               if n.startswith(("FUN_", "thunk_FUN_"))
               and (target, "func_%08X" % a) not in claimed}
    while True:
        grew = False
        for a in sorted(unnamed - sdk):
            who = callers.get(a)
            if who and who <= sdk:
                sdk.add(a)
                grew = True
        if not grew:
            break

    # Rule 4: an unnamed function with a library function on either side of it.
    # The linker lays one object's functions out contiguously, so something
    # sandwiched between two library routines is inside that library - and
    # these have no jal caller at all, being reached through a table, which is
    # why rule 3 cannot see them. The game's own code sits in one run bounded
    # by game functions at both ends, so it is never caught by this.
    order = asm_order(target)
    claimed_here = {a for a in order
                    if (target, "func_%08X" % a) in claimed}
    named = {a for a, n in names.items()
             if not n.startswith(("FUN_", "thunk_FUN_"))}
    def settled(a):
        return a in sdk or a in named or a in claimed_here

    def neighbour(i, step):
        """The nearest function either side that we have already placed.

        Two unclassified stubs sitting next to each other would otherwise each
        wait on the other for ever, so this walks past the unplaced ones.
        """
        j = i + step
        while 0 <= j < len(order) and not settled(order[j]):
            j += step
        return order[j] if 0 <= j < len(order) else None

    for i in range(len(order)):
        a = order[i]
        if settled(a):
            continue
        lo, hi = neighbour(i, -1), neighbour(i, 1)
        if lo in sdk and hi in sdk:
            sdk.add(a)

    sizes = dict(size)
    for i, a in enumerate(order):          # splat knows the ones Ghidra missed
        if a not in sizes:
            sizes[a] = (order[i + 1] - a) if i + 1 < len(order) else 0
    return {a: (names.get(a, ""), sizes.get(a, 0)) for a in sdk}


def main(argv):
    api = psyq_names()
    claimed = decompiled()
    rows = []
    print("%-8s %7s %10s   %7s %10s" % ("target", "sdk", "bytes", "game", "bytes"))
    print("-" * 50)
    for target in EXE:
        _, size = ghidra(target)
        sdk = classify(target, api, claimed)
        for addr, (name, n) in sorted(sdk.items()):
            rows.append((target, addr, n, name))
        sdkb = sum(n for _, n in sdk.values())
        allb = sum(size.values())
        print("%-8s %7d %10d   %7d %10d"
              % (target, len(sdk), sdkb, len(size) - len(sdk), allb - sdkb))

    # The overlays are game code end to end; say so rather than leaving the
    # reader to wonder whether they were simply skipped.
    print("-" * 50)
    print("overlays  no SDK: the signature analyser matches nothing in them")

    if "--dry-run" in argv:
        return 0

    with open(OUT, "w", newline="\n") as f:
        f.write("// Psy-Q SDK functions, written by tools/gen_sdk.py.\n"
                "//\n"
                "// These are linked in from Sony's libraries and must not be\n"
                "// decompiled, so tools/progress.py leaves them out of the\n"
                "// totals and reports them separately. Read the tool's\n"
                "// docstring for how a function ends up here. A row that is\n"
                "// wrong is game code being written off - fix it there.\n"
                "//\n"
                "// target  address   size  name\n")
        for target, addr, n, name in rows:
            f.write("%-8s %08X %6d  %s\n" % (target, addr, n, name))
    print("\nwrote %s (%d functions)" % (os.path.relpath(OUT, ROOT), len(rows)))
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
