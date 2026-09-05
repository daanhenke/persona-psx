"""List functions we have named in Ghidra but have not matched yet.

A name is the expensive half of the work: it means somebody already read the
function and understood what it does. Those are the cheapest decompilations
left, so this is the queue to work from.

    tools/todo_named.py            # every target
    tools/todo_named.py adv btlp   # just these

Columns are target, entry address, size in bytes, and the name.
"""
import json
import os
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
GAME = "p1-jp"
GHIDRA = os.path.join(ROOT, "config", GAME, "ghidra")
DECOMP = os.path.join(ROOT, "config", GAME, "decomp.txt")

# target -> (ghidra json, address space); mirrors tools/gen_names.py
SPACE = {
    "main": ("SLPS_005_00.json", "ram"),
    "atlus": ("ATLUS_EXE.json", "ram"),
    "open": ("OPEN_EXE.json", "ram"),
    "movie": ("MOVIE_EXE.json", "ram"),
    "end": ("END_EXE.json", "ram"),
    "dng": ("SLPS_005_00.json", "OVL_DNG"),
    "btlp": ("SLPS_005_00.json", "OVL_BTLP"),
    "s2d": ("SLPS_005_00.json", "OVL_S2D"),
    "adv": ("SLPS_005_00.json", "OVL_ADV"),
    "casino": ("SLPS_005_00.json", "OVL_CASINO"),
    "name": ("SLPS_005_00.json", "OVL_NAME"),
}


def done():
    """(target, symbol) pairs already recorded as matched."""
    out = set()
    for line in open(DECOMP):
        line = line.split("//")[0].split()
        if len(line) >= 2:
            out.add((line[0], line[1]))
    return out


def named(target):
    jf, space = SPACE[target]
    path = os.path.join(GHIDRA, jf)
    if not os.path.exists(path):
        return []
    sp = json.load(open(path))["spaces"][space]
    user = set(sp.get("user_named", []))
    if not user:
        return []
    # funcs rows are [entry, range_start, range_end]; sum the pieces per entry.
    size = {}
    for ep, lo, hi in sp["funcs"]:
        size[ep] = size.get(ep, 0) + hi - lo + 1
    out = []
    for hexaddr, name in sp.get("names", {}).items():
        if hexaddr not in user:
            continue
        addr = int(hexaddr, 16)
        out.append((addr, size.get(addr, 0), name))
    out.sort(key=lambda r: -r[1])
    return out


def main(argv):
    targets = argv or list(SPACE)
    have = done()
    total = 0
    for target in targets:
        rows = [r for r in named(target)
                if ("%s" % target, "func_%08X" % r[0]) not in have
                and (target, r[2]) not in have]
        if not rows:
            continue
        print("%s  (%d unmatched, %d bytes)"
              % (target, len(rows), sum(r[1] for r in rows)))
        for addr, size, name in rows:
            print("  %08X  %6d  %s" % (addr, size, name))
        total += sum(r[1] for r in rows)
    print("\n%d bytes named but not matched" % total)


if __name__ == "__main__":
    main(sys.argv[1:])
