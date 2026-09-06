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
#include <types.h>

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
    int           col;
    int           i;
    int           j;
    int           sbit;
    int           dbit;
    int           shift;

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
            sbit = i % GLYPH_PER_WORD;
            bit = (*src >> (7 - sbit) & 1) << 1;
            dbit = j % GLYPH_PER_WORD;
            *dst = bit << (dbit * 4) | *dst;
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
    src = (const u_char *)(font + code * GLYPH_BYTES);
    row = 0;
    do {
        col = 0;
        do {
            i = 0;
            do {
                bit = *src >> (7 - i) & 1;
                shift = i * 4;
                bit <<= shift;
                i++;
                if (bit != 0) {
                    *out = bit | ~(3 << shift) & *out;
                }
            } while (i < GLYPH_PER_WORD);
            out++;
            col++;
            src++;
        } while (col < GLYPH_ROW_BYTES);
        row++;
    } while (row < GLYPH_H);
}
