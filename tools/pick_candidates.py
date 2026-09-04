"""Rank functions in the split asm as first-match candidates.

Wants real code, not microleaves: some control flow, at least one call, a
moderate instruction count, and none of the constructs that make a first match
miserable (soft-float, jump tables, multiply/divide sequences).
"""
import json, os, re, sys
from collections import defaultdict

# Ghidra's PsyQ signature analyser names SDK functions (PRNT_OBJ_594, GS_*, ...).
# Those come from the real Psy-Q libraries and must not be decompiled, so only
# functions still carrying a default FUN_ name are game code worth matching.
GHIDRA = os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))),
                      "config", "p1-jp", "ghidra")
SPACE = {
    "main": ("SLPS_005_00.json", "ram"),
    "atlus": ("ATLUS_EXE.json", "ram"), "open": ("OPEN_EXE.json", "ram"),
    "movie": ("MOVIE_EXE.json", "ram"), "end": ("END_EXE.json", "ram"),
    "dng": ("SLPS_005_00.json", "OVL_DNG"), "btlp": ("SLPS_005_00.json", "OVL_BTLP"),
    "s2d": ("SLPS_005_00.json", "OVL_S2D"), "adv": ("SLPS_005_00.json", "OVL_ADV"),
    "casino": ("SLPS_005_00.json", "OVL_CASINO"), "name": ("SLPS_005_00.json", "OVL_NAME"),
}


def ghidra_names(target):
    jf, space = SPACE[target]
    path = os.path.join(GHIDRA, jf)
    if not os.path.exists(path):
        return {}
    return json.load(open(path))["spaces"][space].get("names", {})

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
GAME = "p1-jp"
TARGETS = ["main", "atlus", "open", "movie", "end",
           "dng", "btlp", "s2d", "adv", "casino", "name"]

INSN = re.compile(r"^\s*/\* [0-9A-Fa-f]+ ([0-9A-Fa-f]{8}) [0-9A-Fa-f]{8} \*/\s+(\S+)")
BRANCH = ("beq", "bne", "blez", "bgtz", "bltz", "bgez", "beqz", "bnez", "b ", "bal")
FLOAT = ("mtc1", "mfc1", "cvt", "add.s", "sub.s", "mul.s", "div.s", "lwc1", "swc1",
         "c.lt", "c.le", "c.eq", "cfc1", "ctc1")
GTE = ("mtc2", "mfc2", "cop2", "lwc2", "swc2", "ctc2", "cfc2")
HARD = ("mult", "multu", "div", "divu", "mfhi", "mflo")


def scan(target):
    path = os.path.join(ROOT, "asm", GAME, target, f"{target}.s")
    funcs, cur = [], None
    for line in open(path):
        s = line.strip()
        if s.startswith("glabel "):
            cur = {"name": s.split()[1], "insns": [], "labels": 0,
                   "uses_fp": False, "uses_gp": False}
            funcs.append(cur)
            continue
        if s.startswith("endlabel"):
            cur = None
            continue
        if cur is None:
            continue
        if s.startswith(".L") or s.startswith("alabel"):
            cur["labels"] += 1
        m = INSN.match(line)
        if m:
            if not cur["insns"]:
                cur["vram"] = int(m.group(1), 16)
            cur["insns"].append(m.group(2))
            if "$fp" in line:
                cur["uses_fp"] = True
            if "$gp" in line:
                cur["uses_gp"] = True
    return funcs


MIN_N, MAX_N = 18, 90


def score(f):
    ops = f["insns"]
    n = len(ops)
    if n < MIN_N or n > MAX_N:
        return None
    text = " ".join(ops)
    if any(x in text for x in FLOAT) or any(x in text for x in GTE):
        return None
    # Frame-pointer prologues and $gp-relative accesses mean the translation
    # unit was built with different flags (-O0, non-zero -G) than our -O2 -G0,
    # so those cannot match until per-file flags are supported.
    if f.get("uses_fp") or f.get("uses_gp"):
        return None
    if any(re.search(r"\b%s\b" % x, text) for x in HARD):
        return None
    if "jr" in ops and ops.count("jr") > 1:
        return None                      # likely a jump table
    calls = ops.count("jal") + ops.count("jalr")
    branches = sum(1 for o in ops if any(o.startswith(b.strip()) for b in BRANCH))
    if calls == 0 or branches == 0:
        return None
    mem = sum(1 for o in ops if o.startswith(("lw", "sw", "lh", "sh", "lb", "sb", "lbu", "lhu")))
    # favour balanced functions: real branching, a few calls, some memory work
    return (min(branches, 6) * 3 + min(calls, 5) * 2 + min(mem, 12), n, branches, calls, mem)


def is_game_code(name):
    return name is None or name.startswith("FUN_") or name.startswith("thunk_FUN_")


def main():
    global MIN_N, MAX_N
    per = int(sys.argv[1]) if len(sys.argv) > 1 else 3
    if len(sys.argv) > 3:
        MIN_N, MAX_N = int(sys.argv[2]), int(sys.argv[3])
    for t in TARGETS:
        names = ghidra_names(t)
        cands = []
        for f in scan(t):
            m = re.match(r"func_([0-9A-F]{8})$", f["name"])
            gname = names.get(m.group(1)) if m else None
            if not is_game_code(gname):
                continue
            f["gname"] = gname
            sc = score(f)
            if sc:
                cands.append((sc[0], f, sc))
        cands.sort(key=lambda x: (-x[0], x[1].get("vram", 0)))
        print(f"=== {t}")
        for _, f, sc in cands[:per]:
            print(f"    {f['name']:22} vram=0x{f['vram']:08X} "
                  f"{sc[1]:3} insn  {sc[2]:2} br  {sc[3]:2} call  {sc[4]:2} mem  "
                  f"score={sc[0]}  {f.get('gname') or ''}")
        if not cands:
            print("    (none in range)")


if __name__ == "__main__":
    main()
