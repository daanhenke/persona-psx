/* Persona 1 (JP) - fourteen moves that alternate between two borrowed effects.
 * BTLP only.
 *   0x800BEE48 BtlFxStart92  0x800BEE68 BtlFxStart93  0x800BEE88 BtlFxStart94
 *   0x800BEEA8 BtlFxStart95  0x800BEEC8 BtlFxStart96  0x800BEEE8 BtlFxStart97
 *   0x800BEF08 BtlFxStart98  0x800BEF28 BtlFxStart99  0x800BEF48 BtlFxStart9A
 *   0x800BEF68 BtlFxStart9B  0x800BEF88 BtlFxStart9C  0x800BEFC0 BtlFxStart9D
 *   0x800BEFF8 BtlFxStart9E  0x800BF018 BtlFxStart9F
 *
 * Fourteen start handlers out of g_btl_spell_fx, in seven pairs: the even one
 * of each pair borrows move 0x90's handler and the odd one 0x91's, which are
 * the four-layer and the eight-layer build standing in front of them. The moves
 * are laid out that way because the pair is the same spell at two strengths.
 *
 * The middle pair does one thing more before it calls: the page the effect is
 * drawn from has its semi-transparency rate rewritten, so that spell alone is
 * blended at half rather than at whatever the page was left holding. The rate
 * is set rather than or-ed in, which is why the field is cleared first.
 */
#include <decomp/types.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/object.h>
#include <persona/btlp/spellfx.h>

/* The upload slot these effects are drawn out of, and the two bits of its page
   that say how it is blended - 0x40 is the half-and-half rate. */
#define FX_TPAGE_SLOT 29
#define TPAGE_ABR     0x60
#define TPAGE_ABR_HALF 0x40

BtlObj *BtlFxStart92(void)
{
    return BtlFxStart90();
}

BtlObj *BtlFxStart93(void)
{
    return BtlFxStart91();
}

BtlObj *BtlFxStart94(void)
{
    return BtlFxStart90();
}

BtlObj *BtlFxStart95(void)
{
    return BtlFxStart91();
}

BtlObj *BtlFxStart96(void)
{
    return BtlFxStart90();
}

BtlObj *BtlFxStart97(void)
{
    return BtlFxStart91();
}

BtlObj *BtlFxStart98(void)
{
    return BtlFxStart90();
}

BtlObj *BtlFxStart99(void)
{
    return BtlFxStart91();
}

BtlObj *BtlFxStart9A(void)
{
    return BtlFxStart90();
}

BtlObj *BtlFxStart9B(void)
{
    return BtlFxStart91();
}

BtlObj *BtlFxStart9C(void)
{
    g_btl_tpage[FX_TPAGE_SLOT] =
        (g_btl_tpage[FX_TPAGE_SLOT] & ~TPAGE_ABR) | TPAGE_ABR_HALF;
    return BtlFxStart90();
}

BtlObj *BtlFxStart9D(void)
{
    g_btl_tpage[FX_TPAGE_SLOT] =
        (g_btl_tpage[FX_TPAGE_SLOT] & ~TPAGE_ABR) | TPAGE_ABR_HALF;
    return BtlFxStart91();
}

BtlObj *BtlFxStart9E(void)
{
    return BtlFxStart90();
}

BtlObj *BtlFxStart9F(void)
{
    return BtlFxStart91();
}
