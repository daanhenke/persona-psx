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
#include <decomp/types.h>

#define SEQ_LONG_VALUE 0x80    /* set in the first byte of a two-byte value */
#define STR_END        0xFF
/* Defined in the unit before this one; the prototype is what
   decides how the arguments are converted. */
extern void BtlStrCopyEsc(u_char *dst, const u_char *src, int max);
extern void BtlStrCopy(char *dst, const char *src, int max);


const u_char *BtlSeqReadValue(const u_char *p, u_short *out)
{
    u_char  hi;
    u_short value;

    /* The value goes through a halfword of its own before it is handed back;
       storing straight through the pointer in each arm is a byte short. */
    if (*p < SEQ_LONG_VALUE) {
        value = *p;
        *out = value;
    } else {
        hi = *p;
        p++;
        value = *p | (hi & 0x7F) << 8;
        *out = value;
    }
    return p + 1;
}

/* The same copy with an escape: a byte with the high bit set is written out as
   0x80 and then the byte, so it costs two characters of the limit rather than
   one. That is how the second glyph page is reached. */
