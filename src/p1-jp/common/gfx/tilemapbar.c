/* Persona 1 (JP) - a run of character-map cells with a distinct first cell.
 *
 * Compiled into three overlays rather than called across the boundary:
 *   DNG @ 0x800897B0   ADV @ 0x8007AC34   S2D @ 0x80079BEC
 *
 * Cell 0x19 goes at the destination and 0x1A fills the rest of the width, so
 * a width of one lays down the 0x19 alone. Forty-six call sites in ADV use it,
 * always against a character-map row and always beside text going into the
 * same row, with widths from 2 to 25.
 *
 * The name says what it lays down rather than what it looks like: the two cell
 * ids are the evidence, and the picture they make is not pinned down.
 *
 * The count is a u_char decremented before it is tested, so a width of zero
 * writes the head cell and then 255 more. Nothing calls it that way.
 */
#include <types.h>

#define BAR_HEAD 0x19
#define BAR_BODY 0x1A

void TileMapWriteBar(short *dst, u_char width)
{
    short cell;

    *dst = BAR_HEAD;
    width--;
    if (width == 0) {
        return;
    }
    cell = BAR_BODY;
    do {
        dst++;
        width--;
        *dst = cell;
    } while (width != 0);
}
