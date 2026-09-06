/* Persona 1 (JP) - numbers on the battle screen.
 *   BTLP @ 0x800662F4 BtlFormatDecimal, 0x800AD924 BtlSetGaugeColour
 *
 * BtlFormatDecimal writes digit *values* rather than glyphs - whoever draws
 * them adds the font's base - most significant first, dropping leading zeros
 * unless the caller asks for them. The last place is always written, so zero
 * still prints one digit.
 *
 * BtlSetGaugeColour is the rule the HP and SP bars are tinted by, and it is the
 * same quarter-of-maximum threshold the field's status HUD uses for its danger
 * colour.
 */
#include <types.h>
#include <persona/common/char.h>

/* The long vowel mark stands in for a minus sign; the font has no other. */
#define GLYPH_MINUS 0xCC
#define GLYPH_END   0xFF

#define DECIMAL_TOP 1000000000

/* Tints, two apart, picked by how full the gauge is. */
#define GAUGE_FULL   0x24
#define GAUGE_NORMAL 0x20
#define GAUGE_LOW    0x22

/* Only the tint byte matters here; it sits at +9 of whatever draws the bar. */
typedef struct {
    /* 0x0 */ u_char pad00[9];
    /* 0x9 */ u_char tint;
} BtlGaugeSprite;

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

/* Either sprite may be null: the caller draws only the gauges it has. */
void BtlSetGaugeColour(const Char *c, BtlGaugeSprite *hp,
                       BtlGaugeSprite *sp)
{
    u_char tint;
    int    max;

    if (hp != 0) {
        max = c->hp_max;
        tint = GAUGE_LOW;
        if (max / 4 < c->hp) {
            tint = GAUGE_NORMAL;
            if (c->hp == max) {
                tint = GAUGE_FULL;
            }
        }
        hp->tint = tint;
    }
    if (sp != 0) {
        max = c->sp_max;
        tint = GAUGE_LOW;
        if (max / 4 < c->sp) {
            tint = GAUGE_NORMAL;
            if (c->sp == max) {
                tint = GAUGE_FULL;
            }
        }
        sp->tint = tint;
    }
}
