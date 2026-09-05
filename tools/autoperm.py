#!/usr/bin/env python3
"""Run decomp-permuter on a candidate and re-score what it keeps.

The permuter's own score and objdiff's percentage do not track each other
closely, so its ranking cannot be trusted to pick a winner: this sets a run
going, then puts every output it kept back through tools/mfunc.py and reports
the one objdiff actually likes best.

    tools/autoperm.py <target> <symbol> <cand.c> [seconds] [-jN]

Reach for it when a candidate is at 96-99% with the instruction sequence
already right and only the register assignment differing - that is what the
permuter is for, and grinding declaration orders by hand is not.
"""
import os
import re
import subprocess
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
VENV = os.path.join(ROOT, ".venv/bin/python3")
PERMUTER = os.path.join(ROOT, "tools/decomp-permuter/permuter.py")
SETUP = os.path.join(ROOT, "tools/setup_permuter.py")
MFUNC = os.path.join(ROOT, "tools/mfunc.py")
PCT = re.compile(r"objdiff:\s+([0-9.]+)%")


def score(target, symbol, src):
    r = subprocess.run([VENV, MFUNC, target, symbol, src],
                       capture_output=True, text=True, cwd=ROOT)
    m = PCT.search(r.stdout)
    return float(m.group(1)) if m else 0.0


def main():
    args = [a for a in sys.argv[1:] if not a.startswith("-j")]
    jobs = next((a for a in sys.argv[1:] if a.startswith("-j")), "-j8")
    if len(args) < 3:
        sys.exit(__doc__)
    target, symbol, cand = args[0], args[1], args[2]
    seconds = int(args[3]) if len(args) > 3 else 600

    out = os.path.join(ROOT, "permuter", target, symbol)
    subprocess.run(["rm", "-rf", out])
    subprocess.run([VENV, SETUP, target, symbol, cand], cwd=ROOT)

    base = score(target, symbol, cand)
    print("base %.2f%%  running permuter for %ds" % (base, seconds))
    subprocess.run(["timeout", str(seconds), VENV, PERMUTER, out, jobs,
                    "--stop-on-zero", "--best-only"],
                   capture_output=True, text=True, cwd=ROOT)

    best, best_src = base, cand
    for name in sorted(os.listdir(out)) if os.path.isdir(out) else []:
        src = os.path.join(out, name, "source.c")
        if not name.startswith("output-") or not os.path.exists(src):
            continue
        pct = score(target, symbol, src)
        print("  %-18s %6.2f%%" % (name, pct))
        if pct > best:
            best, best_src = pct, src

    print("best %.2f%%  %s" % (best, os.path.relpath(best_src, ROOT)))
    return 0 if best >= 100.0 else 1


if __name__ == "__main__":
    sys.exit(main())
