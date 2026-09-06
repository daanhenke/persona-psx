"""Score every draft in src/ that config/<game>/decomp.txt does not claim.

A draft is a function that has been written but is not in the match list: either
it never reached 100%, or it did and the row was never added. Both are easy to
lose track of, and the second kind is progress already paid for. This finds them
by compiling each source, reading the function names out of the object, and
asking each target's name map where those names live - so a source that covers
three overlays is checked against all three without anything being spelled out.

    tools/sweep_drafts.py                 # every target, ranked by score
    tools/sweep_drafts.py btlp adv        # only these targets
    tools/sweep_drafts.py --record        # add the exact ones to decomp.txt
    tools/sweep_drafts.py --permute 4     # set the four best near misses going

--record only ever appends rows that objdiff calls exact, so it cannot inflate
progress; run tools/progress.py afterwards to confirm.
"""
import argparse
import os
import re
import shutil
import subprocess
import sys
from concurrent import futures

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from mfunc import (ROOT, GAME, NM, load_target, compile_c, build_target_obj,
                   objdiff_json, pick_symbol, load_names, gp_base,
                   tables_agree, run)

MANIFEST = os.path.join(ROOT, "config", GAME, "decomp.txt")
SRC = os.path.join(ROOT, "src", GAME)
TARGETS = ["main", "atlus", "open", "movie", "end",
           "dng", "btlp", "s2d", "adv", "casino", "name"]


def recorded():
    """(target, symbol) pairs the match list already claims."""
    out = set()
    if not os.path.exists(MANIFEST):
        return out
    for line in open(MANIFEST):
        parts = line.split("//")[0].split()
        if len(parts) >= 3:
            out.add((parts[0], parts[1]))
    return out


FUNCSIZE = re.compile(r"^func_([0-9A-Fa-f]{8}) = 0x[0-9A-Fa-f]+; // type:func")


def defined_funcs(target):
    """Addresses this target actually holds code for.

    A target's name map also carries names for what it calls in the main
    executable, and those are not ours to match here.
    """
    path = os.path.join(ROOT, "config", GAME, "%s.symbols.txt" % target)
    out = set()
    if os.path.exists(path):
        for line in open(path):
            m = FUNCSIZE.match(line.strip())
            if m:
                out.add(int(m.group(1), 16))
    return out


def sources():
    for base, _, files in os.walk(SRC):
        for f in sorted(files):
            if f.endswith(".c"):
                yield os.path.relpath(os.path.join(base, f), ROOT)


def defined_symbols(o_path):
    """Every global text symbol the object defines."""
    out = []
    for line in run([NM, "--defined-only", o_path]).stdout.splitlines():
        p = line.split()
        if len(p) == 3 and p[1] == "T":
            out.append(p[2])
    return out


def build_dir(src, target):
    return os.path.join(ROOT, "build", "sweep",
                        re.sub(r"[^A-Za-z0-9]+", "_", src) + "_" + target)


def compile_for(src, target):
    outdir = build_dir(src, target)
    shutil.rmtree(outdir, ignore_errors=True)
    os.makedirs(outdir)
    return compile_c(os.path.join(ROOT, src), outdir, target)


def probe(src, targets):
    """Function names a source defines, compiling it against whatever builds.

    WORK_BIAS differs per target but the symbol names do not, so one successful
    build is enough to learn what the file holds.
    """
    for t in targets:
        try:
            o = compile_for(src, t)
        except SystemExit:
            continue
        try:
            return defined_symbols(o)
        finally:
            shutil.rmtree(build_dir(src, t), ignore_errors=True)
    return []


def score(target, symbol, cand_o):
    """objdiff percent for one (target, symbol) against an already-built object."""
    vram, orig, lines = load_target(target, symbol)
    outdir = os.path.join(ROOT, "build", "sweep", "_cmp", target, symbol)
    shutil.rmtree(outdir, ignore_errors=True)
    os.makedirs(outdir)
    try:
        names = load_names(target)
        sym = names.get(vram) or symbol
        target_o = build_target_obj(lines, sym, outdir, names, gp_base(target),
                                    target, cand_o)
        d = objdiff_json(target_o, cand_o, sym)
        pct = (pick_symbol((d or {}).get("left"), sym) or {}).get("match_percent")
        if pct is not None and pct >= 100.0 and not tables_agree(target_o, cand_o):
            pct = 0.0
        return pct
    except SystemExit:
        return None
    finally:
        shutil.rmtree(outdir, ignore_errors=True)


def sweep(targets, jobs):
    """Every unrecorded (target, symbol, source) with its score."""
    have = recorded()
    # name -> address, per target; a name we have given something in Ghidra is
    # what ties a source back to the addresses it covers.
    where = {}
    for t in targets:
        own = defined_funcs(t)
        where[t] = {n: a for a, n in load_names(t).items() if a in own}

    found = []

    def one(src):
        names = probe(src, targets)
        if not names:
            return []
        rows = []
        for t in targets:
            wanted = [(n, where[t][n]) for n in names if n in where[t]]
            wanted = [(n, a) for n, a in wanted
                      if (t, "func_%08X" % a) not in have]
            if not wanted:
                continue
            try:
                o = compile_for(src, t)
            except SystemExit:
                continue
            try:
                for n, a in wanted:
                    rows.append((score(t, "func_%08X" % a, o), t,
                                 "func_%08X" % a, n, src))
            finally:
                shutil.rmtree(build_dir(src, t), ignore_errors=True)
        return rows

    with futures.ThreadPoolExecutor(max_workers=jobs) as pool:
        for rows in pool.map(one, sources()):
            found.extend(rows)
    found.sort(key=lambda r: (-(r[0] or 0), r[1], r[2]))
    return found


def record(rows):
    """Append the exact ones to the match list."""
    exact = [r for r in rows if r[0] is not None and r[0] >= 100.0]
    if not exact:
        print("nothing exact to record")
        return
    with open(MANIFEST, "a") as f:
        for _, t, sym, _, src in exact:
            f.write("%-9s %-16s %s\n" % (t, sym, src))
    print("recorded %d exact row(s) to %s" % (len(exact), MANIFEST))


def already_running(t, sym):
    """True if a permuter is already working on this one.

    Re-running the setup under a live permuter would rewrite the base it is
    mutating, so a run in flight is left alone.
    """
    r = subprocess.run(["pgrep", "-f", "permuter/%s/%s" % (t, sym)],
                       capture_output=True, text=True)
    return r.returncode == 0


def permute(rows, count, jobs):
    """Set the best near misses going in the background."""
    near = [r for r in rows if r[0] is not None and r[0] < 100.0]
    near = [r for r in near if not already_running(r[1], r[2])][:count]
    for pct, t, sym, name, src in near:
        subprocess.run([sys.executable,
                        os.path.join(ROOT, "tools", "setup_permuter.py"),
                        t, sym, src], cwd=ROOT)
        log = os.path.join(ROOT, "permuter", t, sym, "run.log")
        with open(log, "w") as f:
            subprocess.Popen(
                [sys.executable,
                 os.path.join(ROOT, "tools", "decomp-permuter", "permuter.py"),
                 os.path.join("permuter", t, sym),
                 "-j%d" % jobs, "--stop-on-zero", "--best-only"],
                cwd=ROOT, stdout=f, stderr=subprocess.STDOUT,
                start_new_session=True)
        print("  permuting %s/%s (%s, %.2f%%) -> %s" % (t, sym, name, pct, log))


def main():
    ap = argparse.ArgumentParser(add_help=False)
    ap.add_argument("targets", nargs="*", default=None)
    ap.add_argument("--record", action="store_true")
    ap.add_argument("--permute", type=int, default=0)
    ap.add_argument("-j", type=int, default=12)
    ap.add_argument("--pj", type=int, default=3)
    ap.add_argument("-h", "--help", action="store_true")
    a = ap.parse_args()
    if a.help:
        sys.exit(__doc__)

    targets = [t for t in (a.targets or TARGETS) if t in TARGETS]
    rows = sweep(targets, a.j)
    if not rows:
        print("no drafts outside the match list")
        return
    for pct, t, sym, name, src in rows:
        print("%7s  %-8s %-16s %-28s %s"
              % ("build!" if pct is None else "%.2f%%" % pct,
                 t, sym, name, src))
    exact = sum(1 for r in rows if r[0] is not None and r[0] >= 100.0)
    print("\n%d draft(s), %d already exact" % (len(rows), exact))
    if a.record:
        record(rows)
    if a.permute:
        permute(rows, a.permute, a.pj)


if __name__ == "__main__":
    main()
