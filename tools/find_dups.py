"""Find the same function compiled into more than one target.

The overlays call into the resident main EXE, but the four sub-EXEs are linked
standalone - so a shared helper is *duplicated* into each of them, at a
different address, with different call targets and different global addresses.
One matching source can therefore cover several targets, which is the cheapest
progress available.

Comparison masks out everything the linker moves:
  - j / jal target fields
  - the 16-bit immediate of address-forming and memory instructions

What survives is the opcode and register stream, which is what actually says
"this is the same code". Branch displacements are kept: they are relative, so
identical code keeps identical branches, and keeping them stops short leaf
functions from colliding by accident.

    tools/find_dups.py                    # every cross-target group
    tools/find_dups.py 40                 # ... at least 40 bytes
    tools/find_dups.py main func_80012B2C # twins of one function
"""
import json
import os
import re
import sys
from collections import defaultdict

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
GAME = "p1-jp"
TARGETS = ["main", "atlus", "open", "movie", "end",
           "dng", "btlp", "s2d", "adv", "casino", "name"]

# Ghidra's PsyQ signature analyser names SDK functions. Those are the most
# duplicated code on the disc and the least useful: they come from the real
# libraries and must never be decompiled.
GHIDRA = os.path.join(ROOT, "config", GAME, "ghidra")
SPACE = {
    "main": ("SLPS_005_00.json", "ram"),
    "atlus": ("ATLUS_EXE.json", "ram"), "open": ("OPEN_EXE.json", "ram"),
    "movie": ("MOVIE_EXE.json", "ram"), "end": ("END_EXE.json", "ram"),
    "dng": ("SLPS_005_00.json", "OVL_DNG"), "btlp": ("SLPS_005_00.json", "OVL_BTLP"),
    "s2d": ("SLPS_005_00.json", "OVL_S2D"), "adv": ("SLPS_005_00.json", "OVL_ADV"),
    "casino": ("SLPS_005_00.json", "OVL_CASINO"), "name": ("SLPS_005_00.json", "OVL_NAME"),
}


def sdk_addrs(target):
    """Addresses the PsyQ signature analyser named - i.e. recognised SDK code.

    Not simply "has a name": every function we match gets named too, and those
    are exactly the ones worth finding twins for. The export records which names
    are USER_DEFINED, so ours survive the filter and the analyser's do not.
    """
    jf, space = SPACE[target]
    path = os.path.join(GHIDRA, jf)
    if not os.path.exists(path):
        return set()
    sp = json.load(open(path))["spaces"][space]
    ours = set(sp.get("user_named", []))
    return {int(a, 16) for a, n in sp.get("names", {}).items()
            if not n.startswith(("FUN_", "thunk_FUN_")) and a not in ours}

LINE = re.compile(r"^\s*/\* ([0-9A-Fa-f]+) ([0-9A-Fa-f]{8}) ([0-9A-Fa-f]{8}) \*/")

# Opcodes whose low 16 bits carry an address the linker chooses.
IMM_OPS = {
    0x08, 0x09,             # addi, addiu
    0x0C, 0x0D, 0x0E,       # andi, ori, xori
    0x0F,                   # lui
    0x20, 0x21, 0x23,       # lb, lh, lw
    0x24, 0x25,             # lbu, lhu
    0x28, 0x29, 0x2B,       # sb, sh, sw
    0x22, 0x26, 0x2A, 0x2E, # lwl, lwr, swl, swr
    0x32, 0x3A,             # lwc2, swc2
}


def mask(word):
    """One instruction, with linker-chosen fields blanked out."""
    op = word >> 26
    if op in (0x02, 0x03):          # j, jal
        return op << 26
    if op in IMM_OPS:
        return word & 0xFFFF0000
    return word


def functions(target):
    """symbol -> (vram, masked bytes tuple) for one target's split asm."""
    path = os.path.join(ROOT, "asm", GAME, target, target + ".s")
    out = {}
    sym = None
    vram = None
    words = []
    for line in open(path):
        s = line.strip()
        if s.startswith("glabel ") or s.startswith("dlabel ") or \
           s.startswith("endlabel "):
            if sym and words:
                out[sym] = (vram, tuple(words))
            sym = s.split()[1] if s.startswith("glabel ") else None
            vram = None
            words = []
            continue
        if sym is None:
            continue
        m = LINE.match(line)
        if m:
            if vram is None:
                vram = int(m.group(2), 16)
            words.append(mask(int(m.group(3)[6:8] + m.group(3)[4:6] +
                                  m.group(3)[2:4] + m.group(3)[0:2], 16)))
    if sym and words:
        out[sym] = (vram, tuple(words))
    return out


def main():
    args = [a for a in sys.argv[1:]]
    minsize = 24
    one = None
    if len(args) == 2:
        one = (args[0], args[1])
    elif len(args) == 1:
        minsize = int(args[0])

    groups = defaultdict(list)
    for t in TARGETS:
        try:
            funcs = functions(t)
        except IOError:
            continue
        sdk = sdk_addrs(t)
        for sym, (vram, words) in funcs.items():
            if len(words) * 4 >= minsize and vram not in sdk:
                groups[words].append((t, sym, vram))

    if one:
        t, sym = one
        key = None
        for words, members in groups.items():
            if any(m[0] == t and m[1] == sym for m in members):
                key = words
                break
        if key is None:
            print("%s/%s: no entry (below the size floor, or not split yet)"
                  % (t, sym))
            return 1
        members = groups[key]
        print("%s/%s  %d bytes" % (t, sym, len(key) * 4))
        for mt, msym, mvram in sorted(members):
            tag = "  <- self" if (mt, msym) == (t, sym) else ""
            print("  %-8s %-16s 0x%08X%s" % (mt, msym, mvram, tag))
        return 0

    multi = [(len(w) * 4, m) for w, m in groups.items()
             if len({x[0] for x in m}) > 1]
    multi.sort(reverse=True)
    print("%d groups spanning more than one target" % len(multi))
    print()
    print("%7s  %-34s %s" % ("bytes", "targets", "symbols"))
    print("-" * 92)
    total = 0
    for size, members in multi:
        targets = sorted({x[0] for x in members})
        total += size * (len(members) - 1)
        print("%7d  %-34s %s"
              % (size, ",".join(targets),
                 " ".join("%s:%s" % (t, s) for t, s, _ in sorted(members))[:120]))
    print("-" * 92)
    print("%d bytes of duplicate code beyond the first copy" % total)
    return 0


if __name__ == "__main__":
    sys.exit(main())
