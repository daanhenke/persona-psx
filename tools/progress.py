"""Report how much of Persona 1 is decompiled and matching.

Reads config/<game>/decomp.txt (target / symbol / source), re-checks each entry
with the same objdiff comparison tools/mfunc.py uses, and measures it against
the total code size taken from the generated symbol files.

    tools/progress.py            # table + totals (uses every core)
    tools/progress.py -j1        # serial, for debugging a build failure
    tools/progress.py --record   # also append a timestamped row to progress.json
                                 # and redraw docs/progress.svg from it
    tools/progress.py --history  # show recorded history

Only functions that actually match count towards progress; a decompiled but
non-matching function is listed separately so it cannot be mistaken for done.
"""
import datetime
import json
from concurrent import futures
import os
import re
import shutil
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import gen_sdk
import progress_chart
from mfunc import (ROOT, GAME, load_target, compile_c, c_symbol,
                   build_target_obj, objdiff_json, pick_symbol, load_names,
                   gp_base, tables_agree)

MANIFEST = os.path.join(ROOT, "config", GAME, "decomp.txt")
SDK = os.path.join(ROOT, "config", GAME, "sdk.txt")
HISTORY = os.path.join(ROOT, "progress.json")
CHART = os.path.join(ROOT, "docs", "progress.svg")
FUNCSIZE = re.compile(r"^(\S+) = 0x([0-9A-Fa-f]+); // type:func size:0x([0-9A-Fa-f]+)")

TARGETS = ["main", "atlus", "open", "movie", "end",
           "dng", "btlp", "s2d", "adv", "casino", "name"]


def total_code(target, sdk):
    """(game bytes, game funcs, sdk bytes, sdk funcs) for one target."""
    path = os.path.join(ROOT, "config", GAME, "%s.symbols.txt" % target)
    if not os.path.exists(path):
        return 0, 0, 0, 0
    skip = sdk.get(target, set())
    total = count = sdkb = sdkn = 0
    for line in open(path):
        m = FUNCSIZE.match(line.strip())
        if not m:
            continue
        size = int(m.group(3), 16)
        if int(m.group(2), 16) in skip:
            sdkb += size
            sdkn += 1
        else:
            total += size
            count += 1
    return total, count, sdkb, sdkn


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


def compile_candidates(pairs, jobs):
    """Compile each distinct (source, target) once, in parallel.

    A source that covers several targets appears once per entry in the
    manifest - tilemap.c alone is twelve - so compiling per entry did the same
    work over and over. The object does depend on the target, though: a shared
    source picks its work area out of the WORK_BIAS define, which is how one
    file serves both the overlay pair and S2D. Most sources ignore it and
    produce the same object for every target; compiling them again per target
    costs a moment and keeps the ones that do not honest.
    """
    out = {}

    def one(key):
        src, target = key
        outdir = os.path.join(ROOT, "build", "progress", "_cand",
                              re.sub(r"[^A-Za-z0-9]+", "_", src) + "_" + target)
        shutil.rmtree(outdir, ignore_errors=True)
        os.makedirs(outdir)
        return compile_c(os.path.join(ROOT, src), outdir, target)

    with futures.ThreadPoolExecutor(max_workers=jobs) as pool:
        pending = {pool.submit(one, k): k for k in pairs}
        for fut in futures.as_completed(pending):
            key = pending[fut]
            try:
                out[key] = fut.result()
            except SystemExit as e:
                print("  ! %s (%s) failed to build: %s" % (key[0], key[1], e))
                out[key] = None
    return out


def check(target, symbol, src, cand_o):
    """Returns (size_bytes, match_percent) for one manifest entry."""
    vram, orig, lines = load_target(target, symbol)
    if cand_o is None:
        return len(orig), None
    outdir = os.path.join(ROOT, "build", "progress", target, symbol)
    shutil.rmtree(outdir, ignore_errors=True)
    os.makedirs(outdir)
    try:
        names = load_names(target)
        sym = c_symbol(cand_o, names.get(vram)) or symbol
        target_o = build_target_obj(lines, sym, outdir, names, gp_base(target),
                                   target, cand_o)
        d = objdiff_json(target_o, cand_o, sym)
        pct = (pick_symbol((d or {}).get("left"), sym) or {}).get("match_percent")
        if pct is not None and pct >= 100.0 and not tables_agree(target_o, cand_o):
            pct = 0.0
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

    jobs = 0
    for a in sys.argv[1:]:
        if a.startswith("-j"):
            jobs = int(a[2:] or 0)
    if jobs <= 0:
        jobs = os.cpu_count() or 4

    cand = compile_candidates(sorted({(e[2], e[0]) for e in entries}), jobs)

    per_target = {}
    rows = [None] * len(entries)

    def one(i):
        target, symbol, src = entries[i]
        size, pct = check(target, symbol, src, cand.get((src, target)))
        rows[i] = (target, symbol, size, pct,
                   pct is not None and pct >= 100.0, src)

    with futures.ThreadPoolExecutor(max_workers=jobs) as pool:
        list(pool.map(one, range(len(entries))))

    for target, symbol, size, pct, matched, _ in rows:
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

    sdk = gen_sdk.load()
    print()
    print("%-8s %10s %10s %8s  %-11s %s"
          % ("target", "matched", "game", "pct", "funcs", "psy-q"))
    print("-" * 72)
    g_matched = g_total = g_funcs = g_tfuncs = 0
    g_sdkb = g_sdkn = 0
    for target in TARGETS:
        total, nfuncs, sdkb, sdkn = total_code(target, sdk)
        got = per_target.get(target, {"bytes": 0, "funcs": 0})
        g_matched += got["bytes"]
        g_total += total
        g_funcs += got["funcs"]
        g_tfuncs += nfuncs
        g_sdkb += sdkb
        g_sdkn += sdkn
        if total:
            print("%-8s %10d %10d %7.3f%%  %-11s %s"
                  % (target, got["bytes"], total,
                     100.0 * got["bytes"] / total,
                     "%d/%d" % (got["funcs"], nfuncs),
                     "%d (%d)" % (sdkb, sdkn) if sdkn else "-"))
    print("-" * 72)
    pct = 100.0 * g_matched / g_total if g_total else 0.0
    print("%-8s %10d %10d %7.3f%%  %-11s %s"
          % ("TOTAL", g_matched, g_total, pct,
             "%d/%d" % (g_funcs, g_tfuncs), "%d (%d)" % (g_sdkb, g_sdkn)))
    print("\npsy-q is linked-in SDK, left out of the totals "
          "(config/%s/sdk.txt)" % GAME)

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
            # Recorded so the step in the graph is readable later: the day the
            # Psy-Q libraries came out of the denominator, the percentage rose
            # without a single function being matched.
            "sdk_bytes": g_sdkb,
            "sdk_funcs": g_sdkn,
            "targets": {t: per_target[t]["bytes"] for t in TARGETS
                        if per_target.get(t, {}).get("bytes")},
        })
        with open(HISTORY, "w", newline="\n") as f:
            json.dump(hist, f, indent=1)
        print("\nrecorded to %s (%d entries)"
              % (os.path.relpath(HISTORY, ROOT), len(hist)))

        svg = progress_chart.build(hist, ROOT, GAME)
        with open(CHART, "w", newline="\n") as f:
            f.write(svg)
        print("drew %s" % os.path.relpath(CHART, ROOT))

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
