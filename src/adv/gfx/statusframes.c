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
#include <decomp/types.h>

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

extern u_short g_menu_bg_rle[];

extern void TileMapBlitRle(u_short *src, short *dst, int stride);
extern void BgBoxShow(void);
extern void DrawStatusHud(void);
extern void DrawPartySlotStatus(int slot, int kind);

/* The cell the use list writes between its columns. */
#define CELL_DIVIDER 0x17

/* The use list's frames: a window and a box across the upper map, the
   menu's backdrop below them, and six ruled rows. The party slots and the
   status HUD go back up as MenuScreenDraw puts them. */
void ItemUseLayout(void)
{
    short *dst;
    short *p;
    u_char row;
    u_char col;
    u_char n;
    short  i;

    dst = &g_tilemap0[0 * W + 7];
    for (row = 0; row < 10; row++) {
        for (col = 0; col < 0x1E; col++) {
            if (row == 0) {
                WindowRowTop(dst, 0x1E);
            } else if (row == 9) {
                WindowRowBottom(dst, 0x1E);
            } else {
                WindowRowMiddle(dst, 0x1E);
            }
        }
        dst += W;
    }
    dst = &g_tilemap0[1 * W + 8];
    for (row = 0; (short)row < 8; row++) {
        for (col = 0; (short)col < 0x1C; col++) {
            do {
                if (row == 0) {
                    BoxRowTop(dst, 0x1C);
                } else if (row == 7) {
                    BoxRowBottom(dst, 0x1C);
                } else {
                    BoxRowMiddle(dst, 0x1C);
                }
            } while (0);
        }
        dst += W;
    }
    TileMapBlitRle(g_menu_bg_rle, &g_tilemap0[10 * W + 0], W);
    for (i = 0; i < 6; i++) {
        p = &g_tilemap0[2 * W + 9 + i * W];
        *p = CELL_DIV;
        n = 9;
        while (n != 0) {
            p++;
            n--;
            *p = CELL_FILL;
        }
        p = &g_tilemap0[2 * W + 19 + i * W];
        *p = CELL_DIV;
        n = 1;
        while (n != 0) {
            p++;
            n--;
            *p = CELL_FILL;
        }
        g_tilemap0[2 * W + 21 + i * W] = CELL_DIVIDER;
        g_tilemap0[2 * W + 22 + i * W] = CELL_DIVIDER;
        /* The bar after the dividers has locals of its own: with p and n
           reused, the two trade registers. */
        {
            short *q = &g_tilemap0[2 * W + 23 + i * W];
            u_char m;

            *q = CELL_DIV;
            m = 9;
            while (m != 0) {
                q++;
                m--;
                *q = CELL_FILL;
            }
        }
        p = &g_tilemap0[2 * W + 33 + i * W];
        *p = CELL_DIV;
        n = 1;
        while (n != 0) {
            p++;
            n--;
            *p = CELL_FILL;
        }
    }
    BgBoxShow();
    DrawStatusHud();
    DrawPartySlotStatus(0, 0);
    DrawPartySlotStatus(1, 0);
    DrawPartySlotStatus(2, 0);
    DrawPartySlotStatus(3, 0);
    DrawPartySlotStatus(4, 0);
}
