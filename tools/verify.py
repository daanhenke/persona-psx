"""Compare each built target against the original disc binary."""
import hashlib, os, sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
GAME = "p1-jp"
DATA = f"scratch/extracted/{GAME}"

ORIG = {
    "main": "SLPS_005.00", "atlus": "EXE/ATLUS.EXE", "open": "EXE/OPEN.EXE",
    "movie": "EXE/MOVIE.EXE", "end": "EXE/END.EXE",
    "dng": "DNG.BIN", "btlp": "BTLP.BIN", "s2d": "S2D.BIN",
    "adv": "ADV.BIN", "casino": "CASINO.BIN", "name": "NAME.BIN",
}


def sha1(b):
    return hashlib.sha1(b).hexdigest()


def main():
    names = sys.argv[1:] or list(ORIG)
    print(f"{'target':9} {'built':>9} {'original':>9} {'delta':>7}  result")
    print("-" * 68)
    bad = 0
    for t in names:
        bp = os.path.join(ROOT, "build", GAME, f"{t}.bin")
        op = os.path.join(ROOT, DATA, ORIG[t])
        if not os.path.exists(bp):
            print(f"{t:9} {'-':>9} {'-':>9} {'-':>7}  NOT BUILT")
            bad = 1
            continue
        b = open(bp, "rb").read()
        o = open(op, "rb").read()
        delta = len(b) - len(o)
        if b == o:
            print(f"{t:9} {len(b):>9} {len(o):>9} {delta:>7}  MATCH")
            continue
        bad = 1
        n = min(len(b), len(o))
        first = next((i for i in range(n) if b[i] != o[i]), None)
        if first is None and delta > 0 and not any(b[len(o):]):
            note = f"MATCH + {delta}B zero padding"
        elif first is None:
            note = f"prefix matches, size differs by {delta}"
        else:
            note = f"differs at 0x{first:X} ({100.0*first/len(o):.1f}% in)"
        print(f"{t:9} {len(b):>9} {len(o):>9} {delta:>7}  {note}")
    print("-" * 68)
    print("all match" if not bad else "FAILURES PRESENT")
    return bad


if __name__ == "__main__":
    sys.exit(main())
