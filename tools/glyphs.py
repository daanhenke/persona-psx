#!/usr/bin/env python3
"""Read the game's text out of a target's tile font.

TileMapWriteRow expands one packed byte per character-map cell, adding a base
so the caller picks the glyph bank, so a label in the data is a run of glyph
indices ending at 0xFF (or padded with 0, the blank cell). The font puts A-Z at
0xA6..0xBF and 0-9 at 0xC0..0xC9; two independent words settle it, 0xA9AAB1AAB9AA
reading DELETE and 0xA8ADA6B3ACAAB8B9A6B9BAB8 reading CHANGESTATUS.

The kana below that came out of the name-entry keyboard, which is a table of
glyph codes laid out on screen in gojuon order - see KANA. Between them the two
cover the whole font, so a label anywhere in the game now reads as text rather
than as a run of hex.

    tools/glyphs.py <target> <addr> [count]   # decode one label
    tools/glyphs.py <target> --scan [minlen]  # every label in the target
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


# The kana half of the font, read off the name-entry keyboard.
#
# g_keyboard in the NAME overlay is four pages of six rows of ten keys, drawn
# in the order they appear on screen, and the screen order is gojuon. Page 0
# gives hiragana 0x01..0x2E and the small kana at 0x48..0x50; page 1 the same
# for katakana at 0x52 and 0x9A; page 2 is the Latin page, and its rows put A
# at 0xA6 and 0 at 0xC0 exactly where they were already known to be.
#
# The keyboard has no keys for the voiced forms - they are reached with a
# modifier - so those are filled in by continuing the same order, which leaves
# exactly the 25 slots between "n" and the small vowels in both halves. Four
# strings confirm it end to end: the config screen reads BATTLE SETTINGS,
# COMMAND CONFIRM, MESSAGE SPEED, WINDOW ANIMATION, and its value labels read
# NORMAL / FAST / OFF and DO / DO NOT.
#
# Katakana carries one glyph hiragana does not, at 0x80, which is what puts
# every voiced katakana one slot later than the offset alone would predict -
# without it MESSAGE SPEED comes out a character wrong. It is almost certainly
# vu, the one katakana-only voiced form.
_BASE = ("あいうえお"      # a i u e o
         "かきくけこ"      # ka ki ku ke ko
         "さしすせそ"      # sa shi su se so
         "たちつてと"      # ta chi tsu te to
         "なにぬねの"      # na ni nu ne no
         "はひふへほ"      # ha hi fu he ho
         "まみむめも"      # ma mi mu me mo
         "やゆよ"                  # ya yu yo
         "らりるれろ"      # ra ri ru re ro
         "わをん")                 # wa wo n
_VOICED = ("がぎぐげご"    # ga gi gu ge go
           "ざじずぜぞ"    # za ji zu ze zo
           "だぢづでど"    # da ji zu de do
           "ばびぶべぼ"    # ba bi bu be bo
           "ぱぴぷぺぽ")   # pa pi pu pe po
_SMALL = "ぁぃぅぇぉ"      # small a i u e o
_TSU_YA = "っゃゅょ"           # small tsu ya yu yo

# code -> character. Hiragana runs from 0x01, katakana from 0x52 as the same
# sequence shifted into the katakana block, with the extra glyph at 0x80.
KANA = {}
for _i, _c in enumerate(_BASE + _VOICED):
    KANA[0x01 + _i] = _c
for _i, _c in enumerate(_SMALL):
    KANA[0x48 + _i] = _c
KANA[0x4D] = _TSU_YA[0]
for _i, _c in enumerate(_TSU_YA[1:]):
    KANA[0x4E + _i] = _c


def _kata(c):
    """Hiragana to katakana; the blocks are 0x60 apart in Unicode."""
    return chr(ord(c) + 0x60)


for _i, _c in enumerate(_BASE):
    KANA[0x52 + _i] = _kata(_c)
KANA[0x80] = "ヴ"                          # vu - the extra slot
for _i, _c in enumerate(_VOICED):
    KANA[0x81 + _i] = _kata(_c)
for _i, _c in enumerate(_SMALL):
    KANA[0x9A + _i] = _kata(_c)
KANA[0xA1] = _kata(_TSU_YA[0])
for _i, _c in enumerate(_TSU_YA[1:]):
    KANA[0xA2 + _i] = _kata(_c)

# Punctuation, from the keyboard's own keys. 0xCC is the long-vowel bar: it is
# what makes the message-speed label read as a word.
KANA[0xCC] = "ー"


def glyph(b, blank=" "):
    if 0xA6 <= b <= 0xBF:
        return chr(ord("A") + b - 0xA6)
    if 0xC0 <= b <= 0xC9:
        return chr(ord("0") + b - 0xC0)
    if b in KANA:
        return KANA[b]
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
