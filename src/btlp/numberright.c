/* Persona 1 (JP) - three ways of laying a number out.  BTLP only.
 *   0x800AC928 BtlFormatHex  0x800ACA20 BtlFormatRight  0x800ACA9C BtlHexDigits
 *
 * BtlFormatRight is the one the drawing pass uses: digits right aligned in a
 * fixed-width field, blanks to the left, a minus once if there was one. The
 * last place is always written so zero still shows a digit.
 *
 * The other two are a pair. BtlHexDigits spells the value in ASCII hex,
 * backwards from the end of the field, and BtlFormatHex runs it and then
 * converts what it wrote into font cells - so unlike the decimal path, the
 * scratch field here holds characters rather than digit values.
 */
#include <decomp/types.h>
#include <persona/btlp/number.h>

/* Turns a value into the cells that spell it in hex, terminated. The blank
   test cannot fire: BtlHexDigits fills every place it is given. */
void BtlFormatHex(u_char *dst, int value, int digits)
{
    /* Eight bytes of frame nothing here uses, and the function is the wrong
       length without them. */
    u_long      scratch[2];
    signed char c;
    int         wide;
    signed char ch;
    int         i;
    int         cell;

    BtlHexDigits(g_btl_number_buf, value, digits);
    for (i = 0; i < digits; i++) {
        c = g_btl_number_buf[i];
        if (c == NUMBER_BLANK) {
            *dst = GLYPH_SPACE;
        } else {
            /* Through a word and back: the original keeps the character in a
               second register, and it is the copy that fills the branch's
               delay slot. One step is not enough - gcc coalesces that. */
            wide = c;
            ch = wide;
            if ((u_int)(ch - 'A') < HEX_LETTERS) {
                cell = ch + HEX_LETTER_BASE;
            } else {
                cell = ch + HEX_DIGIT_BASE;
            }
            *dst = cell;
        }
        dst++;
    }
    *dst = GLYPH_END;
}

void BtlFormatRight(signed char *dst, int value, int width)
{
    int rest;
    int neg;

    neg = 0;
    if (width <= 0) {
        width = 1;
    }
    if (value < 0) {
        value = -value;
        neg = 1;
    }
    dst[width - 1] = value % 10;
    rest = value / 10;
    if (width >= 2) {
        width -= 2;
        if (width >= 0) {
            do {
                if (rest != 0) {
                    dst[width] = rest % 10;
                    rest /= 10;
                } else if (neg) {
                    dst[width] = NUMBER_MINUS;
                    neg = 0;
                } else {
                    dst[width] = NUMBER_BLANK;
                }
                width--;
            } while (width >= 0);
        }
    }
}

/* Written from the last place backwards, so the value is consumed a nibble at
   a time and no length has to be worked out first. */
void BtlHexDigits(signed char *dst, int value, int digits)
{
    signed char *p;
    int          nibble;
    int          c;

    digits--;
    if (digits < 0) {
        return;
    }
    p = (signed char *)(digits + (int)dst);
    do {
        nibble = value - (value / 16) * 16;
        c = nibble + '0';
        /* Between the two, not after them: the division has to land in the
           branch's delay slot for the registers to come out as the original
           has them. */
        value /= 16;
        if ((u_char)c >= '9' + 1) {
            c = nibble + ('A' - 10);
        }
        *p = c;
        digits--;
        p--;
    } while (digits >= 0);
}
