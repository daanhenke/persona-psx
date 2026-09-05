/* Persona 1 (JP) - the status screen's frames and rules.  ADV @ 0x8007B9BC.
 *
 * Three frames in the lower character map - an ornate window, a plain box
 * inside it, and a second window beside them - and then rows of divider and
 * fill cells ruling them into columns.
 *
 * The frame loops are the bodies of TileMapDrawWindow and TileMapDrawBox
 * written out rather than called, which is why their bounds differ: the window
 * pair tests unsigned counts and the box pair signed ones, exactly as the two
 * callable routines do. Keep the casts and the do/while - both are load-bearing.
 */
#include <types.h>

/* The lower of the two 40x64 character-map layers ADV clears. */
#define g_tilemap0 ((short *)0x800EE180)
#define W 40

#define CELL_DIV  0x19   /* the left edge of a ruled span */
#define CELL_FILL 0x1A

extern void WindowRowTop(short *dst, u_char w);
extern void WindowRowMiddle(short *dst, u_char w);
extern void WindowRowBottom(short *dst, u_char w);
extern void BoxRowTop(short *dst, u_char w);
extern void BoxRowMiddle(short *dst, u_char w);
extern void BoxRowBottom(short *dst, u_char w);

void DrawStatusFrames(void)
{
    short *dst;
    short *p;
    u_char row;
    u_char col;
    u_char n;
    short  i;

    dst = &g_tilemap0[8 * W + 17];
    for (row = 0; row < 10; row++) {
        for (col = 0; col < 0x16; col++) {
            if (row == 0) {
                WindowRowTop(dst, 0x16);
            } else if (row == 9) {
                WindowRowBottom(dst, 0x16);
            } else {
                WindowRowMiddle(dst, 0x16);
            }
        }
        dst += W;
    }
    dst = &g_tilemap0[9 * W + 18];
    for (row = 0; (short)row < 8; row++) {
        for (col = 0; (short)col < 0x14; col++) {
            do {
                if (row == 0) {
                    BoxRowTop(dst, 0x14);
                } else if (row == 7) {
                    BoxRowBottom(dst, 0x14);
                } else {
                    BoxRowMiddle(dst, 0x14);
                }
            } while (0);
        }
        dst += W;
    }
    dst = &g_tilemap0[7 * W + 2];
    for (row = 0; row < 0xB; row++) {
        for (col = 0; col < 0xE; col++) {
            if (row == 0) {
                WindowRowTop(dst, 0xE);
            } else if (row == 0xA) {
                WindowRowBottom(dst, 0xE);
            } else {
                WindowRowMiddle(dst, 0xE);
            }
        }
        dst += W;
    }
    for (i = 0; i < 7; i++) {
        p = &g_tilemap0[10 * W + 3 + i * W];
        *p = CELL_DIV;
        n = 1;
        while (n != 0) {
            p++;
            n--;
            *p = CELL_FILL;
        }
        p = &g_tilemap0[10 * W + 5 + i * W];
        *p = CELL_DIV;
        n = 9;
        while (n != 0) {
            p++;
            n--;
            *p = CELL_FILL;
        }
    }
    for (i = 0; i < 5; i++) {
        p = &g_tilemap0[11 * W + 19 + i * W];
        *p = CELL_DIV;
        n = 2;
        while (n != 0) {
            p++;
            n--;
            *p = CELL_FILL;
        }
        p = &g_tilemap0[11 * W + 22 + i * W];
        *p = CELL_DIV;
        n = 1;
        while (n != 0) {
            p++;
            n--;
            *p = CELL_FILL;
        }
        p = &g_tilemap0[11 * W + 24 + i * W];
        *p = CELL_DIV;
        n = 1;
        while (n != 0) {
            p++;
            n--;
            *p = CELL_FILL;
        }
    }
    for (i = 0; i < 6; i++) {
        p = &g_tilemap0[10 * W + 27 + i * W];
        *p = CELL_DIV;
        n = 4;
        while (n != 0) {
            p++;
            n--;
            *p = CELL_FILL;
        }
        p = &g_tilemap0[10 * W + 32 + i * W];
        *p = CELL_DIV;
        n = 1;
        while (n != 0) {
            p++;
            n--;
            *p = CELL_FILL;
        }
        p = &g_tilemap0[10 * W + 34 + i * W];
        *p = CELL_DIV;
        n = 2;
        while (n != 0) {
            p++;
            n--;
            *p = CELL_FILL;
        }
    }
}
