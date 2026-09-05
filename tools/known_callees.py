"""Rank unmatched functions by how much of what they call is already named.

A function whose callees all have real names can be read: the SDK calls say
what it talks to, and our own names say what it does with it. One whose callees
are all still bare addresses can only be guessed at, and a guessed name is
worse than none. This sorts the queue accordingly.

    tools/known_callees.py btlp              # every unmatched function
    tools/known_callees.py btlp 40 600       # only 40..600 bytes
    tools/known_callees.py btlp --min 0.75   # only well-understood ones

Columns: address, size, the fraction of its calls that land somewhere named,
its own name if it has one, and the callees themselves.
"""
import json
import os
import re
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
GAME = "p1-jp"
GHIDRA = os.path.join(ROOT, "config", GAME, "ghidra")
DECOMP = os.path.join(ROOT, "config", GAME, "decomp.txt")
ASM = os.path.join(ROOT, "asm", GAME)

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

GLABEL = re.compile(r"^glabel (\S+)")
JAL = re.compile(r"^\s*/\* \S+ \S+ \S+ \*/\s+jal\s+(\S+)")
ADDR = re.compile(r"^(?:func|D)_([0-9A-Fa-f]{8})$")


def names(target):
    """address -> name, plus the subset we named ourselves.

    An overlay calls into the resident EXE, so main's names count too: that is
    where the SDK actually lives.
    """
    jf, space = SPACE[target]
    doc = json.load(open(os.path.join(GHIDRA, jf)))
    spaces = [doc["spaces"][space]]
    if space.startswith("OVL_"):
        spaces.append(doc["spaces"]["ram"])
    out, ours = {}, set()
    for sp in spaces:
        user = set(sp.get("user_named", []))
        for hexaddr, name in sp.get("names", {}).items():
            if name.startswith(("FUN_", "thunk_FUN_")):
                continue
            out[int(hexaddr, 16)] = name
            if hexaddr in user:
                ours.add(int(hexaddr, 16))
    return out, ours


def calls(target):
    """symbol -> [size in bytes, callee symbols in call order]."""
    path = os.path.join(ASM, target, target + ".s")
    out, cur, n = {}, None, 0
    for line in open(path):
        m = GLABEL.match(line)
        if m:
            cur, n = m.group(1), 0
            out[cur] = [0, []]
            continue
        if cur is None:
            continue
        if line.startswith("endlabel"):
            out[cur][0] = n * 4
            cur = None
            continue
        if line.lstrip().startswith("/*"):
            n += 1
            m = JAL.match(line)
            if m:
                out[cur][1].append(m.group(1))
    return out


def done():
    out = set()
    for line in open(DECOMP):
        f = line.split("//")[0].split()
        if len(f) >= 2:
            out.add((f[0], f[1]))
    return out


def sdk(target):
    """Addresses of the Psy-Q functions in this target (tools/gen_sdk.py).

    Without this the listing for main and the sub-EXEs is nothing but library
    code, which is neither ours to write nor useful to rank.
    """
    sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
    import gen_sdk
    return gen_sdk.load().get(target, set())


def main(argv):
    target = argv[0]
    lo, hi, floor = 0, 1 << 30, 0.0
    rest = argv[1:]
    if "--min" in rest:
        i = rest.index("--min")
        floor = float(rest[i + 1])
        rest = rest[:i] + rest[i + 2:]
    if len(rest) >= 1:
        lo = int(rest[0])
    if len(rest) >= 2:
        hi = int(rest[1])

    name, ours = names(target)
    have = done()
    skip = sdk(target)
    rows = []
    for sym, (size, callees) in calls(target).items():
        if (target, sym) in have or not (lo <= size <= hi):
            continue
        m = ADDR.match(sym)
        if not m:
            continue
        addr = int(m.group(1), 16)
        if addr in skip:
            continue
        resolved = []
        for c in callees:
            cm = ADDR.match(c)
            resolved.append(name.get(int(cm.group(1), 16)) if cm else c)
        known = [c for c in resolved if c]
        frac = 1.0 if not resolved else len(known) / float(len(resolved))
        if frac < floor:
            continue
        seen, uniq = set(), []
        for c in known:
            if c not in seen:
                seen.add(c)
                uniq.append(c)
        rows.append((frac, size, addr, name.get(addr, ""), len(resolved), uniq))

    rows.sort(key=lambda r: (-r[0], -r[1]))
    for frac, size, addr, own, ncall, uniq in rows:
        print("%08X %6d  %3d%% of %-2d  %-24s %s"
              % (addr, size, round(frac * 100), ncall, own or "-",
                 " ".join(uniq[:8])))
    print("\n%d functions" % len(rows))


if __name__ == "__main__":
    main(sys.argv[1:])
