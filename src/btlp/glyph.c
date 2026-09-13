/* Persona 1 (JP) - one glyph from 1bpp into 4bpp.  BTLP only.
 *   0x80066948 BtlExpandGlyph
 *
 * A glyph on the disc is 0x20 bytes: sixteen rows of sixteen bits. The message
 * windows want it as 4bpp pixels, eight to a word, and drawn with a drop
 * shadow - so this makes two passes over the same bits.
 *
 * The first lays the shadow down as colour 2, offset one row down and one
 * pixel right, which is why it starts two words in and begins its destination
 * pixel at 1 rather than 0. The second writes the ink over it as colour 1,
 * clearing both bits of the pixel it lands on so the shadow does not show
 * through.
 *
 * The two colours are the ones g_btl_text_cluts holds at indices 1 and 2 - the
 * ink and the dark blue behind it - with 0 left transparent.
 */
#include <decomp/types.h>

/* A glyph as it arrives, and as it is drawn. */
#define GLYPH_BYTES 0x20
#define GLYPH_W     16
#define GLYPH_H     16

/* Pixels to a word at 4bpp, and bytes to a row at 1bpp. */
#define GLYPH_PER_WORD 8
#define GLYPH_ROW_BYTES 2

void BtlExpandGlyph(int code, u_int *dst, int font)
{
    const u_char *src;
    u_int        *out;
    u_int         bit;
    int           row;
    u_char        bits;
    int           i;
    int           j;
    int           sbit;
    int           dbit;

    /* The shadow, a row down and a pixel across. */
    src = (const u_char *)(font + code * GLYPH_BYTES);
    out = dst;
    *dst = 0;
    dst++;
    *dst = 0;
    dst++;
    row = 0;
    do {
        *dst = 0;
        j = 1;
        i = 0;
        do {
            bits = *src;
            sbit = i - i / GLYPH_PER_WORD * GLYPH_PER_WORD;
            bit = (bits >> (7 - sbit) & 1) << 1;
            dbit = j - j / GLYPH_PER_WORD * GLYPH_PER_WORD;
            bit <<= dbit * 4;
            *dst = bit | *dst;
            if (sbit == 7) {
                src++;
            }
            j++;
            if (dbit == 7) {
                dst++;
                *dst = 0;
            }
            i++;
        } while (j < GLYPH_W);
        row++;
        src++;
    } while (row < GLYPH_H - 1);

    /* The ink over it. */
    dst = out;
    src = (const u_char *)(font + code * GLYPH_BYTES);
    row = 0;
    do {
        i = 0;
        do {
            j = 0;
            do {
                bits = *src;
                bits >>= 7 - j;
                bit = bits & 1;
                bit <<= j * 4;
                if (bit != 0) {
                    *dst = bit | ~(3 << (j * 4)) & *dst;
                }
                j++;
            } while (j < GLYPH_PER_WORD);
            dst++;
            i++;
            src++;
        } while (i < GLYPH_ROW_BYTES);
        row++;
    } while (row < GLYPH_H);
}
