#!/usr/bin/env python3
"""Score several candidate spellings of one function at once.

Guessing which C shape gcc 2.6 will reproduce is faster done in parallel than
one edit at a time: write four or five variants, run them all, keep the one
that scores highest and iterate from there. Register allocation and induction
variables in particular respond to spellings that are hard to predict from the
diff alone.

    tools/tryv.py <target> <symbol> variants/*.c
    tools/tryv.py --diff <target> <symbol> variants/*.c   # diff the winner

Prints one line per variant, best first.
"""
import os
import re
import subprocess
import sys
from concurrent.futures import ThreadPoolExecutor

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
MFUNC = os.path.join(ROOT, "tools", "mfunc.py")
PCT = re.compile(r"objdiff:\s*([0-9.]+)%")


def score(target, symbol, cfile, tag):
    env = dict(os.environ, MFUNC_TAG=tag, PYTHONDONTWRITEBYTECODE="1")
    try:
        out = subprocess.run([sys.executable, MFUNC, target, symbol, cfile],
                             capture_output=True, text=True, env=env,
                             cwd=ROOT).stdout
    except Exception as exc:                      # noqa: BLE001
        return cfile, -1.0, str(exc)
    m = PCT.search(out)
    if not m:
        tail = [l for l in out.splitlines() if l.strip()]
        return cfile, -1.0, tail[-1] if tail else "no output"
    return cfile, float(m.group(1)), ""


def main():
    args = sys.argv[1:]
    want_diff = False
    if args and args[0] == "--diff":
        want_diff, args = True, args[1:]
    if len(args) < 3:
        print(__doc__.strip())
        return 2
    target, symbol, files = args[0], args[1], args[2:]

    with ThreadPoolExecutor(max_workers=min(8, len(files))) as pool:
        rows = list(pool.map(
            lambda a: score(target, symbol, a[1], "-v%d" % a[0]),
            enumerate(files)))

    rows.sort(key=lambda r: -r[1])
    width = max(len(os.path.basename(f)) for f, _, _ in rows)
    for cfile, pct, err in rows:
        name = os.path.basename(cfile).ljust(width)
        if pct < 0:
            print("  %s  ----     %s" % (name, err))
        else:
            mark = "  EXACT MATCH" if pct == 100.0 else ""
            print("  %s  %6.2f%%%s" % (name, pct, mark))

    if want_diff and rows and rows[0][1] >= 0:
        print()
        subprocess.run([sys.executable, MFUNC, target, symbol, rows[0][0],
                        "--diff"], cwd=ROOT)
    return 0


if __name__ == "__main__":
    sys.exit(main())
