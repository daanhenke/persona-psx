/* Persona 1 (JP) - what a message substitutes into itself.  BTLP only.
 *   0x80079ED8 BtlSetInsert
 *
 * Seven slots of 0x18 bytes, filled with a name, a number, whose turn it is.
 * Filling one sets the bit that makes BtlTextOpen restart the window: the
 * buffer's address has not changed but what it holds has, and the window would
 * otherwise leave the old message running.
 *
 * A number goes in as glyph pairs - an attribute byte and the glyph - because
 * that is what the window's cells take. BtlFormatDecimal hands back digit
 * values, and taking 0x40 off a byte lands on the font's digits at 0xC0.
 *
 * The case numbers do not run in slot order. Only the calls know which is
 * which, so the switch is written out rather than indexed.
 */
#include <types.h>

#define BTL_TEXT_EDITED 1

/* An insert slot, and the two lengths the copies are capped at. */
#define INSERT_SHORT 8
#define INSERT_LONG  10

/* Glyph pairs: the attribute byte, then the glyph. */
#define INSERT_ATTR 0x80
#define DIGIT_BASE  (-0x40)
#define INSERT_END  0xFF

typedef struct {
    /* 0x00 */ u_char cell[0x18];
} BtlInsert;                        /* 0x18 bytes */

extern BtlInsert g_btl_insert[];
extern int       g_btl_text_edited;

extern u_char *BtlFormatDecimal(int value, u_char *dst, int pad);
extern void    BtlStrCopyEsc(u_char *dst, const u_char *src, int max);
extern void    BtlStrCopy(u_char *dst, const u_char *src, int max);

void BtlSetInsert(int which, const u_char *src)
{
    BtlInsert *slot;
    u_char *dst;
    u_char *q;
    int     attr;
    int     end;
    u_char  buf[16];

    slot = g_btl_insert;
    g_btl_text_edited |= BTL_TEXT_EDITED;
    switch (which) {
    case 0:
        BtlFormatDecimal((int)src, buf, 0);
        q = buf;
        dst = slot[0].cell;
        attr = INSERT_ATTR;
        end = INSERT_END;
        while (*q != end) {
            dst[0] = attr;
            dst[1] = *q + DIGIT_BASE;
            q++;
            dst += 2;
        }
        *dst = INSERT_END;
        break;
    case 1:
        BtlStrCopyEsc(slot[1].cell, src, INSERT_SHORT);
        break;
    case 2:
        BtlStrCopyEsc(slot[3].cell, src, INSERT_LONG);
        break;
    case 3:
        BtlStrCopyEsc(slot[4].cell, src, INSERT_LONG);
        break;
    case 4:
        BtlStrCopyEsc(slot[6].cell, src, INSERT_SHORT);
        break;
    case 5:
        BtlStrCopyEsc(slot[5].cell, src, INSERT_LONG);
        break;
    case 6:
        BtlStrCopy(slot[2].cell, src, INSERT_LONG);
        break;
    }
}
