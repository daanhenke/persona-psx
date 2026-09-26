/* Persona 1 (JP) - a font glyph into a 4bpp strip.  NAME @ 0x80067E24.
 *
 * The same expansion as the field's FieldDecodeGlyph, but into any strip
 * `stride` words wide rather than the scene's own cell, so a row of keys can
 * be built side by side and uploaded at once.
 */
#include <decomp/types.h>
#include <persona/common/font.h>

/* The low word's top five bits, carried into the high word. The US source
   masks them before shifting, which gcc keeps as an extra and. */
#ifdef VER_US
#define CARRY(c) (((c) & 0xF8000000) >> 27)
#else
#define CARRY(c) ((c) >> 27)
#endif

/* Expands glyph `code` bottom row first, two words a row, and drops a shadow
   one pixel right and down by or'ing each row, shifted five bits, into the
   row below it. */
void ExpandGlyph(u_short code, u_long *dst, int stride)
{
    u_char *src;
    int     row, half, k;
    u_long  words[2];
    u_int   bits;

    src = g_font_bits + 31 + code * 32;
    for (row = 15; row >= 0; row--) {
        words[1] = 0;
        words[0] = 0;
        for (half = 1; half >= 0; half--) {
            bits = *src--;
            for (k = 7; k >= 0; k--) {
                words[half] |= (bits & 1) << (k * 4);
                bits >>= 1;
            }
        }
        dst[row * stride] = words[0];
        dst[row * stride + 1] = words[1];
        if (row < 15) {
            u_long c = dst[row * stride];

            dst[(row + 1) * stride] |= c << 5;
            dst[(row + 1) * stride + 1] |= (dst[row * stride + 1] << 5) | CARRY(c);
        }
    }
}
