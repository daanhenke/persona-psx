/* Persona 1 (JP) - the rest of the background cell-map writers.
 *
 * Compiled into three overlays rather than called across the boundary, and
 * byte-identical in all three because nothing here touches a global:
 *              DNG         ADV         S2D
 *   WriteRowR  0x800773E4  0x80067C1C  0x8006740C
 *   FillRect   0x80077430  0x80067C68  0x80067458
 *   BlitRle    0x8007748C  0x80067CC4  0x800674B4
 *
 * The forward writer these share their shape with is a unit of its own ahead
 * of them, in tilemap.c.
 */
#include <decomp/types.h>

/* Same, walking the destination backwards - used for right-aligned numbers. */
void TileMapWriteRowRev(const u_char *src, short *dst, int base, u_short count)
{
    while (count != 0) {
        if (*src == 0xFF) {
            return;
        }
        *dst = *src + base;
        src++;
        count--;
        dst--;
    }
}

/* Fills a w*h rectangle of cells with one value. `stride` is the layer width in
   cells - 40 for every layer the game sets up - so the step from the end of one
   row to the start of the next is stride - w cells. */
void TileMapFillRect(short *dst, short value, u_short w, u_short h,
                     u_short stride)
{
    int x, y;

    for (y = 0; y < h; y++) {
        for (x = 0; x < w; x++) {
            *dst = value;
            dst++;
        }
        dst -= w;
        dst += stride;
    }
}

/* Run-length blit into a rectangle. The source begins with its own width and
   height as two u16s, then alternating (value, run) pairs; a run carries across
   the end of a row, so the encoding is of the rectangle as a whole and not of
   each line. `stride` is the destination layer's width in cells. */
void TileMapBlitRle(const u_short *src, short *dst, u_short stride)
{
    unsigned int w, h;
    unsigned int x, y;
    u_short value, run;

    run = 0;
    w = *src++;
    h = *src++;

    for (y = 0; y < h; y++) {
        for (x = 0; x < w; x++) {
            if (run == 0) {
                value = *src++;
                run = *src++;
            }
            *dst++ = value;
            run--;
        }
        dst += stride;
        dst -= w;
    }
}
