/* Persona 1 (JP) - decimal formatting for the character-map layers.
 *
 * Compiled into three overlays rather than called across the boundary:
 *   DNG @ 0x80076048
 *   ADV @ 0x800665C4
 *   S2D @ 0x8006605C
 * Each overlay carries its own copy of g_pow10, so the addresses differ but
 * the code does not.
 */
#include <types.h>

/* The ten powers of ten, 1 through 1000000000: digit i of a `width`-digit
   field is g_pow10[i - 1], so the table is indexed one entry below the digit
   number. */
extern const u_int g_pow10[];

/* Writes `width` digit bytes, least significant first, and returns how many of
   them are significant (at least 1). The buffer is little-endian by digit
   because TileMapWriteRowRev consumes it that way: it walks the destination
   cells backwards while reading these forwards, which puts the units digit in
   the rightmost column.
 *
 * The final scan walks the buffer again to find the highest non-zero digit,
 * which is what the caller uses to suppress the leading zeroes. A value of 0
 * still counts as one digit. */
short FormatDecimal(u_int value, u_char *dst, u_short width)
{
    u_short i;
    u_short last;

    last = 0;
    for (i = 0; i < width; i++) {
        *dst = 0;
        dst++;
    }

    /* i == width here, and the digits are emitted from the most significant
       end downwards, so the pointer walks back as the divisor shrinks. */
    dst--;
    if (i != 0) {
        do {
            if (value >= g_pow10[i - 1]) {
                *dst = value / g_pow10[i - 1];
                value = value % g_pow10[i - 1];
            }
            i--;
            dst--;
        } while (i != 0);
    }

    for (i = 0; i < width; i++) {
        dst++;
        if (*dst != 0) {
            last = i;
        }
    }
    return last + 1;
}
