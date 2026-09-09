/* Persona 1 (JP) - a packed byte string into background cell indices.
 *
 * Compiled into three overlays rather than called across the boundary, and
 * byte-identical in all three because nothing here touches a global:
 *   DNG 0x80077344   ADV 0x80067B7C   S2D 0x8006736C
 *
 * The layers are grids of 16-bit cell indices, 40 columns wide - 40 cells of
 * 8 pixels is the 320-pixel screen. ADV clears three of them at 0x800EE180
 * (40x64), 0x800EF580 (40x64) and 0x800F0980 (40x32); the first two are
 * 0x1400 bytes apart, which is exactly 40*64*2, so they are contiguous.
 *
 * The reverse writer and the two rectangle routines are a unit of their own in
 * tilemapblit.c: a routine none of the three overlays has worked out sits
 * between this and them.
 */
#include <decomp/types.h>

/* Expands a packed byte string into cell indices, adding a base so the caller
   picks which bank of glyphs the bytes name. 0xFF terminates early - it is the
   "no cell here" code, not a value that can be biased.

   The caller that made the purpose obvious formats a number into digit bytes
   and then calls the reverse variant below, which lays the digits out from the
   right so the least significant one lands in a fixed column. */
void TileMapWriteRow(const u_char *src, short *dst, int base, u_short count)
{
    while (count != 0) {
        if (*src == 0xFF) {
            return;
        }
        *dst = *src + base;
        src++;
        count--;
        dst++;
    }
}
