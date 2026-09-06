/* Persona 1 (JP) - how far an effect is shifted when it is drawn.  BTLP only.
 *   0x80079178 BtlEffectOffset
 *
 * Two bits of the effect's flags choose the scale: four times the shorts at
 * +0x18 and +0x1A, eight times them, or no shift at all. Both come out negated,
 * so the shift is away from whatever those two describe. The effect drawer
 * calls this immediately before drawing, and the two globals are what it reads.
 */
#include <types.h>
#include <persona/btlp/effect.h>

extern short g_btl_effect_ox;
extern short g_btl_effect_oy;

void BtlEffectOffset(const BtlEffect *e)
{
    switch ((e->flags >> BTL_EFFECT_SHIFT) & 3) {
    case BTL_EFFECT_SHIFT_NONE:
        g_btl_effect_ox = 0;
        g_btl_effect_oy = 0;
        break;
    case BTL_EFFECT_SHIFT_4X:
        g_btl_effect_ox = e->dx * -4;
        g_btl_effect_oy = -(e->dy << 2);
        break;
    case BTL_EFFECT_SHIFT_8X:
        g_btl_effect_ox = e->dx * -8;
        g_btl_effect_oy = -(e->dy << 3);
        break;
    }
}
