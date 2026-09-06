/* Persona 1 (JP) - putting a number on the battle screen.
 *   BTLP @ 0x800AC878 BtlDrawNumber, 0x800AC7C8 BtlDrawNumberAlt
 *
 * BtlFormatRight lays the digits out right aligned in a scratch buffer, using
 * 0xFF for a blank and 0xFE for a minus, and these turn that field into glyphs.
 * A digit becomes 0xC0 + d, which is where the font keeps 0 to 9; the minus
 * becomes 0xCC, the long vowel mark, the font having nothing better; a blank
 * becomes glyph 0.
 *
 * The two differ only in the digit base. The second set at 0xE5 is the other
 * size of digit the overlay carries.
 *
 * Both return how many characters were actually drawn - blanks do not count -
 * so a caller can centre what it has just laid out.
 */
#include <types.h>

#define NUMBER_BLANK (-1)
#define NUMBER_MINUS (-2)

#define GLYPH_MINUS  0xCC
#define GLYPH_SPACE  0

/* d - 0x40 lands on 0xC0 + d, and d - 0x1B on 0xE5 + d. */
#define DIGIT_BASE     (-0x40)
#define DIGIT_BASE_ALT (-0x1B)

extern signed char g_btl_number_buf[];

extern void BtlFormatRight(signed char *dst, int value, int width);

int BtlDrawNumber(u_char *dst, int value, int width)
{
    signed char c;
    int         i;
    int         drawn;

    BtlFormatRight(g_btl_number_buf, value, width);
    i = 0;
    drawn = 0;
    if (width > 0) {
        do {
            c = g_btl_number_buf[i];
            if (c != NUMBER_BLANK) {
                if (c == NUMBER_MINUS) {
                    *dst = GLYPH_MINUS;
                } else {
                    *dst = c + DIGIT_BASE;
                }
                drawn++;
            } else {
                *dst = GLYPH_SPACE;
            }
            i++;
            dst++;
        } while (i < width);
    }
    *dst = -1;
    return drawn;
}

int BtlDrawNumberAlt(u_char *dst, int value, int width)
{
    signed char c;
    int         i;
    int         drawn;

    BtlFormatRight(g_btl_number_buf, value, width);
    i = 0;
    drawn = 0;
    if (width > 0) {
        do {
            c = g_btl_number_buf[i];
            if (c != NUMBER_BLANK) {
                if (c == NUMBER_MINUS) {
                    *dst = GLYPH_MINUS;
                } else {
                    *dst = c + DIGIT_BASE_ALT;
                }
                drawn++;
            } else {
                *dst = GLYPH_SPACE;
            }
            i++;
            dst++;
        } while (i < width);
    }
    *dst = -1;
    return drawn;
}
