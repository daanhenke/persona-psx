"""Report how much of Persona 1 is decompiled and matching.

Reads config/<game>/decomp.txt (target / symbol / source), re-checks each entry
with the same objdiff comparison tools/mfunc.py uses, and measures it against
the total code size taken from the generated symbol files.

    tools/progress.py            # table + totals
    tools/progress.py --record   # also append a timestamped row to progress.json
    tools/progress.py --history  # show recorded history

Only functions that actually match count towards progress; a decompiled but
non-matching function is listed separately so it cannot be mistaken for done.
"""
import datetime
import json
import os
import re
import shutil
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from mfunc import (ROOT, GAME, load_target, compile_c, c_symbol,
                   build_target_obj, objdiff_json, pick_symbol, load_names,
                   gp_base)

MANIFEST = os.path.join(ROOT, "config", GAME, "decomp.txt")
HISTORY = os.path.join(ROOT, "progress.json")
FUNCSIZE = re.compile(r"type:func size:0x([0-9A-Fa-f]+)")

TARGETS = ["main", "atlus", "open", "movie", "end",
           "dng", "btlp", "s2d", "adv", "casino", "name"]


def total_code(target):
    """Bytes of code in a target, from the generated symbol file."""
    path = os.path.join(ROOT, "config", GAME, "%s.symbols.txt" % target)
    if not os.path.exists(path):
        return 0, 0
    total = 0
    count = 0
    for line in open(path):
        m = FUNCSIZE.search(line)
        if m:
            total += int(m.group(1), 16)
            count += 1
    return total, count


def read_manifest():
    entries = []
    if not os.path.exists(MANIFEST):
        return entries
    for line in open(MANIFEST):
        line = line.split("//")[0].strip()
        if not line:
            continue
        parts = line.split()
        if len(parts) >= 3:
            entries.append((parts[0], parts[1], parts[2]))
    return entries


def check(target, symbol, src):
    """Returns (size_bytes, match_percent) for one manifest entry."""
    vram, orig, lines = load_target(target, symbol)
    outdir = os.path.join(ROOT, "build", "progress", target, symbol)
    shutil.rmtree(outdir, ignore_errors=True)
    os.makedirs(outdir)
    try:
        names = load_names(target)
        cand_o = compile_c(os.path.join(ROOT, src), outdir)
        sym = c_symbol(cand_o, names.get(vram)) or symbol
        target_o = build_target_obj(lines, sym, outdir, names, gp_base(target))
        d = objdiff_json(target_o, cand_o, sym)
        pct = (pick_symbol((d or {}).get("left"), sym) or {}).get("match_percent")
    except SystemExit as e:
        print("  ! %s/%s failed to build: %s" % (target, symbol, e))
        pct = None
    finally:
        shutil.rmtree(outdir, ignore_errors=True)
    return len(orig), pct


def main():
    entries = read_manifest()
    if not entries:
        print("no entries in %s" % os.path.relpath(MANIFEST, ROOT))
        return 0

    per_target = {}
    rows = []
    for target, symbol, src in entries:
        size, pct = check(target, symbol, src)
        matched = pct is not None and pct >= 100.0
        rows.append((target, symbol, size, pct, matched, src))
        t = per_target.setdefault(target, {"bytes": 0, "funcs": 0})
        if matched:
            t["bytes"] += size
            t["funcs"] += 1

    print("%-8s %-18s %7s  %8s  %s" % ("target", "symbol", "bytes", "objdiff", "state"))
    print("-" * 66)
    for target, symbol, size, pct, matched, _ in rows:
        state = "MATCH" if matched else "in progress"
        pcts = "%.2f%%" % pct if pct is not None else "  error"
        print("%-8s %-18s %7d  %8s  %s" % (target, symbol, size, pcts, state))

    print()
    print("%-8s %10s %10s %8s  %s" % ("target", "matched", "total", "pct", "funcs"))
    print("-" * 60)
    g_matched = g_total = g_funcs = g_tfuncs = 0
    for target in TARGETS:
        total, nfuncs = total_code(target)
        got = per_target.get(target, {"bytes": 0, "funcs": 0})
        g_matched += got["bytes"]
        g_total += total
        g_funcs += got["funcs"]
        g_tfuncs += nfuncs
        if total:
            print("%-8s %10d %10d %7.3f%%  %d/%d"
                  % (target, got["bytes"], total,
                     100.0 * got["bytes"] / total, got["funcs"], nfuncs))
    print("-" * 60)
    pct = 100.0 * g_matched / g_total if g_total else 0.0
    print("%-8s %10d %10d %7.3f%%  %d/%d"
          % ("TOTAL", g_matched, g_total, pct, g_funcs, g_tfuncs))

    if "--record" in sys.argv:
        hist = []
        if os.path.exists(HISTORY):
            try:
                hist = json.load(open(HISTORY))
            except ValueError:
                hist = []
        hist.append({
            "date": datetime.datetime.now().strftime("%Y-%m-%dT%H:%M:%S"),
            "matched_bytes": g_matched,
            "total_bytes": g_total,
            "percent": round(pct, 4),
            "matched_funcs": g_funcs,
            "total_funcs": g_tfuncs,
        })
        with open(HISTORY, "w", newline="\n") as f:
            json.dump(hist, f, indent=1)
        print("\nrecorded to %s (%d entries)"
              % (os.path.relpath(HISTORY, ROOT), len(hist)))

    if "--history" in sys.argv and os.path.exists(HISTORY):
        print()
        print("%-20s %10s %8s %s" % ("date", "bytes", "pct", "funcs"))
        for h in json.load(open(HISTORY)):
            print("%-20s %10d %7.3f%% %d/%d"
                  % (h["date"], h["matched_bytes"], h["percent"],
                     h["matched_funcs"], h["total_funcs"]))
    return 0


if __name__ == "__main__":
    sys.exit(main())
