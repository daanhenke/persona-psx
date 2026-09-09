/* Persona 1 (JP) - a number into digits.
 *   BTLP @ 0x800662F4 BtlFormatDecimal
 *
 * Writes digit *values* rather than glyphs - whoever draws them adds the
 * font's base - most significant first, dropping leading zeros unless the
 * caller asks for them. The last place is always written, so zero still
 * prints one digit.
 *
 * The right-aligned form is a unit of its own much later in the overlay, in
 * numberright.c, and the gauge tint later still in gaugecolour.c.
 */
#include <decomp/types.h>

/* The long vowel mark stands in for a minus sign; the font has no other. */
#define GLYPH_MINUS 0xCC
#define GLYPH_END   0xFF

#define DECIMAL_TOP 1000000000

u_char *BtlFormatDecimal(int value, u_char *dst, int pad)
{
    u_int place;
    u_int digit;

    place = DECIMAL_TOP;
    if (value < 0) {
        /* Spelt out rather than -value: the original complements and adds. */
        value = ~value + 1;
        *dst = GLYPH_MINUS;
        dst++;
    }
    do {
        digit = (u_int)value / place;
        if (digit != 0 || pad != 0 || place == 1) {
            pad = 1;
            *dst = digit % 10;
            dst++;
        }
        place /= 10;
    } while (place != 0);
    *dst = GLYPH_END;
    return dst;
}
