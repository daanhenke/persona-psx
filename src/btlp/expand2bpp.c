/* Persona 1 (JP) - one 2bpp cel into 4bpp.  BTLP only.
 *   0x800668C0 BtlExpand2bpp
 *
 * Sixteen rows of sixteen pixels, four to a source byte and four to a
 * destination halfword, so a cel is 0x40 bytes in and 0x80 out. Each two-bit
 * pixel is widened to a nibble in place - the palette entries line up, so
 * nothing is remapped on the way through.
 *
 * The sibling in glyph.c does the same job from 1bpp and draws a shadow while
 * it is there; this one is the plain widening, and nothing calls it.
 */
#include <decomp/types.h>

/* Rows to a cel, destination halfwords to a row, and pixels to one of those. */
#define CEL_ROWS   16
#define CEL_WORDS  4
#define CEL_PIXELS 4

/* Bytes a source cel takes, and the two field widths. */
#define CEL_BYTES  0x40
#define CEL_IN     2
#define CEL_OUT    4
#define CEL_MASK   3

void BtlExpand2bpp(int index, u_short *dst, const u_char *src)
{
    int     row;
    int     col;
    int     k;
    int     mask;
    u_short bits;

    src += index * CEL_BYTES;
    row = 0;
    /* Through a variable: the original keeps the mask in a register across all
       three loops rather than rebuilding it. */
    mask = CEL_MASK;
    do {
        col = 0;
        do {
            *dst = 0;
            k = 0;
            do {
                bits = (*src & (mask << (k * CEL_IN))) >> (k * CEL_IN);
                *dst = (bits << (k * CEL_OUT)) | *dst;
                k++;
            } while (k < CEL_PIXELS);
            dst++;
            col++;
            src++;
        } while (col < CEL_WORDS);
        row++;
    } while (row < CEL_ROWS);
}
