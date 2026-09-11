/* Persona 1 (JP) - fourteen moves in the middle of the effect table.
 * BTLP only.
 *   0x800BDD18 BtlFxStart76  0x800BDD38 BtlFxStart77  0x800BDD58 BtlFxStart78
 *   0x800BDD80 BtlFxStart79  0x800BDDA8 BtlFxStart7A  0x800BDDD4 BtlFxStart7B
 *   0x800BDE00 BtlFxStart7C  0x800BDE2C BtlFxStart7D  0x800BDE58 BtlFxStart7E
 *   0x800BDE84 BtlFxStart7F  0x800BDEB0 BtlFxStart80  0x800BDEDC BtlFxStart81
 *   0x800BDF08 BtlFxStart82  0x800BDF34 BtlFxStart83
 *
 * Fourteen start handlers out of g_btl_spell_fx. The first two borrow the
 * handler of move 0x75, which stands immediately in front of them; the next two
 * put a single record on the fighter aimed at; the ten after that are the same
 * record over everything the move reaches, walked to grey.
 */
#include <decomp/types.h>
#include <persona/btlp/object.h>
#include <persona/btlp/spellfx.h>

#define FX_TIMER 0
#define FX_GREY  0x80

BtlObj *BtlFxStart76(void)
{
    return BtlFxStart75();
}

BtlObj *BtlFxStart77(void)
{
    return BtlFxStart75();
}

BtlObj *BtlFxStart78(void)
{
    return BtlOpenFxObj2(g_btl_fx_target, FX_TIMER);
}

BtlObj *BtlFxStart79(void)
{
    return BtlOpenFxObj2(g_btl_fx_target, FX_TIMER);
}

BtlObj *BtlFxStart7A(void)
{
    return BtlOpenFxOnTargets(FX_GREY, FX_GREY, FX_GREY, FX_TIMER);
}

BtlObj *BtlFxStart7B(void)
{
    return BtlOpenFxOnTargets(FX_GREY, FX_GREY, FX_GREY, FX_TIMER);
}

BtlObj *BtlFxStart7C(void)
{
    return BtlOpenFxOnTargets(FX_GREY, FX_GREY, FX_GREY, FX_TIMER);
}

BtlObj *BtlFxStart7D(void)
{
    return BtlOpenFxOnTargets(FX_GREY, FX_GREY, FX_GREY, FX_TIMER);
}

BtlObj *BtlFxStart7E(void)
{
    return BtlOpenFxOnTargets(FX_GREY, FX_GREY, FX_GREY, FX_TIMER);
}

BtlObj *BtlFxStart7F(void)
{
    return BtlOpenFxOnTargets(FX_GREY, FX_GREY, FX_GREY, FX_TIMER);
}

BtlObj *BtlFxStart80(void)
{
    return BtlOpenFxOnTargets(FX_GREY, FX_GREY, FX_GREY, FX_TIMER);
}

BtlObj *BtlFxStart81(void)
{
    return BtlOpenFxOnTargets(FX_GREY, FX_GREY, FX_GREY, FX_TIMER);
}

BtlObj *BtlFxStart82(void)
{
    return BtlOpenFxOnTargets(FX_GREY, FX_GREY, FX_GREY, FX_TIMER);
}

BtlObj *BtlFxStart83(void)
{
    return BtlOpenFxOnTargets(FX_GREY, FX_GREY, FX_GREY, FX_TIMER);
}
