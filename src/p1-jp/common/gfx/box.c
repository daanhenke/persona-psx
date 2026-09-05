/* Persona 1 (JP) - window and box frames in the character-map layers.
 *
 * Two frame styles, drawn a row at a time. The ornate one uses tiles 1..0xD
 * and has two-cell corners; the plain one uses 0xE..0x16 and single-cell caps.
 *
 * Compiled into three overlays rather than called across the boundary:
 *                     DNG         ADV         S2D
 *   WindowRowTop      0x80089604  0x8007AA88  0x80079A40
 *   WindowRowMiddle   0x800896A0  0x8007AB24  0x80079ADC
 *   WindowRowBottom   0x80089710  0x8007AB94  0x80079B4C
 *   TileMapDrawWindow 0x800894FC  0x8007A980  0x80079938
 *   BoxRowTop         0x800898FC  0x8007AD80  0x80079D38
 *   BoxRowMiddle      0x8008996C  0x8007ADF0  0x80079DA8
 *   BoxRowBottom      0x800899DC  0x8007AE60  0x80079E18
 *   TileMapDrawBox    0x800897E8  0x8007AC6C  0x80079C24
 */
#include <types.h>

/* Ornate frame, top row: 1 and 2 at the left, 4 and 5 at the right, 3 across
   the middle. */
void WindowRowTop(short *dst, u_char w)
{
    u_char i;

    for (i = 0; i < w; i++) {
        if (i == 0) {
            dst[0] = 1;
        } else if (i == 1) {
            dst[1] = 2;
        } else if (i == w - 1) {
            dst[i] = 5;
        } else if (i == w - 2) {
            dst[i] = 4;
        } else {
            dst[i] = 3;
        }
    }
}

/* Ornate frame, middle row: single-cell caps 6 and 8 around a run of 7. */
void WindowRowMiddle(short *dst, u_char w)
{
    u_char i;

    for (i = 0; i < w; i++) {
        if (i == 0) {
            dst[0] = 6;
        } else if (i == w - 1) {
            dst[i] = 8;
        } else {
            dst[i] = 7;
        }
    }
}

/* Ornate frame, bottom row: the same five-piece shape as the top, tiles
   9..0xD. */
void WindowRowBottom(short *dst, u_char w)
{
    u_char i;

    for (i = 0; i < w; i++) {
        if (i == 0) {
            dst[0] = 9;
        } else if (i == 1) {
            dst[1] = 0xA;
        } else if (i == w - 1) {
            dst[i] = 0xD;
        } else if (i == w - 2) {
            dst[i] = 0xC;
        } else {
            dst[i] = 0xB;
        }
    }
}

/* Plain frame: left cap, run, right cap, three tiles per row. */
void BoxRowTop(short *dst, u_char w)
{
    u_char i;

    for (i = 0; i < w; i++) {
        if (i == 0) {
            dst[0] = 0xE;
        } else if (i == w - 1) {
            dst[i] = 0x10;
        } else {
            dst[i] = 0xF;
        }
    }
}

void BoxRowMiddle(short *dst, u_char w)
{
    u_char i;

    for (i = 0; i < w; i++) {
        if (i == 0) {
            dst[0] = 0x11;
        } else if (i == w - 1) {
            dst[i] = 0x13;
        } else {
            dst[i] = 0x12;
        }
    }
}

void BoxRowBottom(short *dst, u_char w)
{
    u_char i;

    for (i = 0; i < w; i++) {
        if (i == 0) {
            dst[0] = 0x14;
        } else if (i == w - 1) {
            dst[i] = 0x16;
        } else {
            dst[i] = 0x15;
        }
    }
}

/* Draws a frame w by h cells, stepping `stride` cells between rows.
 *
 * The inner loop redraws the whole row once per column, so a w-wide frame
 * draws each row w times over. That is what the original does. */
void TileMapDrawWindow(short *dst, u_char w, u_char h, u_char stride)
{
    u_char row;
    u_char col;

    for (row = 0; row < h; row++) {
        for (col = 0; col < w; col++) {
            if (row == 0) {
                WindowRowTop(dst, w);
            } else if (row == h - 1) {
                WindowRowBottom(dst, w);
            } else {
                WindowRowMiddle(dst, w);
            }
        }
        dst += stride;
    }
}

void TileMapDrawBox(short *dst, u_short w, short h, u_short stride)
{
    u_char row;
    u_char col;

    for (row = 0; row < h; row++) {
        for (col = 0; col < (short)w; col++) {
            if (row == 0) {
                BoxRowTop(dst, w);
            } else if (row == h - 1) {
                BoxRowBottom(dst, w);
            } else {
                BoxRowMiddle(dst, w);
            }
        }
        dst += (short)stride;
    }
}
