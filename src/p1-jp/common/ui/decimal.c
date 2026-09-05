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

/* 1, 10, ... 1000000000. Indexed from one entry *before* the array: digit i
   uses g_pow10[i - 1], and gcc folds the -1 into the base rather than the
   index, so the linked binary builds &g_pow10[-1] and the disassembler blames
   whatever object precedes the table. normalize_asm in tools/mfunc.py knows
   about that case. */
extern const u_int g_pow10[];

/* Writes `width` digit bytes, least significant first, and returns how many of
   them are significant (at least 1). The buffer is little-endian by digit
   because TileMapWriteRowRev consumes it that way: it walks the destination
   cells backwards while reading these forwards, which puts the units digit in
   the rightmost column.
 *
 * The scan at the end advances the pointer *before* testing, which is what
   lets the second loop leave it one short of the buffer and never correct
   afterwards - writing it as a trailing `dst--` costs the match.
 */
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
