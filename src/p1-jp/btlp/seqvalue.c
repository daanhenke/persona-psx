/* Persona 1 (JP) - two small pieces of the battle script reader.
 *   BTLP @ 0x8007B468 BtlSeqReadValue, 0x80079E8C BtlStrCopy
 *
 * Script values are one byte where they can be: anything under 0x80 stands for
 * itself, and anything else is the low seven bits of that byte joined to the
 * one after it. So a value up to 0x7F costs a byte and one up to 0x7FFF costs
 * two, and the reader hands back where it stopped.
 *
 * Strings in the same data are terminated by 0xFF rather than zero - that is
 * the font's own end marker, since 0 is a real glyph.
 */
#include <types.h>

#define SEQ_LONG_VALUE 0x80    /* set in the first byte of a two-byte value */
#define STR_END        0xFF

const u_char *BtlSeqReadValue(const u_char *p, u_short *out)
{
    u_char hi;

    if (*p < SEQ_LONG_VALUE) {
        *out = *p;
    } else {
        hi = *p;
        p++;
        *out = *p | (hi & 0x7F) << 8;
    }
    return p + 1;
}

/* Stops at `max` characters and terminates the destination either way. */
void BtlStrCopy(char *dst, const char *src, int max)
{
    char c;
    int  i;

    c = *src;
    for (i = 0; c != STR_END && i < max; i++) {
        c = *src;
        src++;
        *dst = c;
        c = *src;
        dst++;
    }
    *dst = STR_END;
}
