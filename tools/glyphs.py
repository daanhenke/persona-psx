#!/usr/bin/env python3
"""Read the game's Latin text out of a target's tile font.

TileMapWriteRow expands one packed byte per character-map cell, adding a base
so the caller picks the glyph bank, so a label in the data is a run of glyph
indices ending at 0xFF (or padded with 0, the blank cell). The font puts A-Z at
0xA6..0xBF and 0-9 at 0xC0..0xC9; two independent words settle it, 0xA9AAB1AAB9AA
reading DELETE and 0xA8ADA6B3ACAAB8B9A6B9BAB8 reading CHANGESTATUS.

    tools/glyphs.py <target> <addr> [count]   # decode one label
    tools/glyphs.py <target> --scan [minlen]  # every label in the target

Most of the text is Japanese and outside this range; the scan is for the parts
that are not, which is enough to name the tables they sit in.
"""
import re
import sys

import os

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
GAME = "p1-jp"

# Where each target's image loads, and how much of the file precedes it.
LOAD = {
    "main": (0x80010000, 0x800), "atlus": (0x80080000, 0x800),
    "open": (0x80080000, 0x800), "movie": (0x80080000, 0x800),
    "end": (0x80080000, 0x800), "dng": (0x800643A0, 0), "btlp": (0x800643A0, 0),
    "s2d": (0x800643A0, 0), "adv": (0x800643A0, 0), "casino": (0x800643A0, 0),
    "name": (0x800643A0, 0),
}


def glyph(b, blank=" "):
    if 0xA6 <= b <= 0xBF:
        return chr(ord("A") + b - 0xA6)
    if 0xC0 <= b <= 0xC9:
        return chr(ord("0") + b - 0xC0)
    if b == 0:
        return blank
    return None


def image(target):
    load, skip = LOAD[target]
    path = os.path.join(ROOT, "build", GAME, target + ".bin")
    if not os.path.exists(path):
        sys.exit("%s not built; run make first" % os.path.relpath(path, ROOT))
    return open(path, "rb").read(), load, skip


def show(target, addr, count):
    data, load, skip = image(target)
    raw = data[addr - load + skip:addr - load + skip + count]
    print("%08X: %s" % (addr, " ".join("%02X" % b for b in raw)))
    print("          " + "".join(glyph(b) or "{%02X}" % b for b in raw))


def scan(target, minlen):
    data, load, skip = image(target)
    run, start = [], None
    for i in range(skip, len(data)):
        c = glyph(data[i])
        if c is not None:
            if start is None:
                start = i
            run.append(c)
            continue
        text = "".join(run).strip()
        if start is not None and len(re.sub(r"[ 0-9]", "", text)) >= minlen:
            print("%08X  %s" % (load + start - skip, text))
        run, start = [], None


def main():
    if len(sys.argv) < 3:
        sys.exit(__doc__)
    target = sys.argv[1]
    if sys.argv[2] == "--scan":
        scan(target, int(sys.argv[3]) if len(sys.argv) > 3 else 4)
        return 0
    show(target, int(sys.argv[2], 16),
         int(sys.argv[3]) if len(sys.argv) > 3 else 32)
    return 0


if __name__ == "__main__":
    sys.exit(main())
