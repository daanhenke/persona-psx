/* Persona 1 (JP) - a number appended to a character-map row.
 *
 *   ADV @ 0x8007D864   S2D @ 0x8007C264
 *
 * Two bytes go down per digit: a 0x80 and then the digit's glyph, which the
 * font keeps at 0xC0 for zero. Digit positions the value did not need get a
 * single blank each, so a short number sits right-aligned in its field without
 * the caller having to measure it.
 *
 * The digits come back from FormatDecimal least significant first, so they are
 * read from the top of the buffer downwards to come out in reading order.
 *
 * The end of what was written is handed back, which is how a caller builds a
 * row of several numbers in one pass.
 */
#include <types.h>

/* The font's zero, and the byte that goes in front of every digit. */
#define GLYPH_DIGIT0 0xC0
#define GLYPH_LEAD   0x80

/* A digit position the number did not reach. */
#define GLYPH_BLANK 0

extern u_char g_hud_digits[];

extern short FormatDecimal(u_int value, u_char *dst, u_short width);

u_char *TextAppendNumber(u_char *dst, short value, u_char width)
{
    int n;

    n = FormatDecimal(value, g_hud_digits, width);
    while (n < width) {
        *dst = GLYPH_BLANK;
        dst++;
        width--;
    }
    if (n != 0) {
        do {
            *dst = GLYPH_LEAD;
            dst++;
            *dst = g_hud_digits[n - 1] + GLYPH_DIGIT0;
            n--;
            dst++;
        } while (n != 0);
    }
    return dst;
}
