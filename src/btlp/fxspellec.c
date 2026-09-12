/* Persona 1 (JP) - the last seven handlers in the effect table.  BTLP only.
 *   0x800BFC94 BtlFxStartEC  0x800BFCB4 BtlFxStartED  0x800BFCDC BtlFxStartF1
 *   0x800BFD04 BtlFxStartF3  0x800BFD2C BtlFxStartF4  0x800BFD58 BtlFxStartF5
 *   0x800BFD84 BtlFxStartF6
 *
 * Seven start handlers out of g_btl_spell_fx and the last of them in the image:
 * a grid over the field, four single records on the fighter aimed at, and two
 * over everything the move reaches walked to grey. As with the block before
 * them their ids do not run in order - the file keeps the image's.
 */
#include <decomp/types.h>
#include <persona/btlp/object.h>
#include <persona/btlp/spellfx.h>

BtlObj *BtlFxStartEC(void)
{
    return BtlOpenFxGrid(FX_GRID_TABLE);
}

BtlObj *BtlFxStartED(void)
{
    return BtlOpenFxObj2(g_btl_fx_target, FX_TIMER);
}

BtlObj *BtlFxStartF1(void)
{
    return BtlOpenFxObj2(g_btl_fx_target, FX_TIMER);
}

BtlObj *BtlFxStartF3(void)
{
    return BtlOpenFxObj2(g_btl_fx_target, FX_TIMER);
}

BtlObj *BtlFxStartF4(void)
{
    return BtlOpenFxOnTargets(FX_GREY, FX_GREY, FX_GREY, FX_TIMER);
}

BtlObj *BtlFxStartF5(void)
{
    return BtlOpenFxOnTargets(FX_GREY, FX_GREY, FX_GREY, FX_TIMER);
}

BtlObj *BtlFxStartF6(void)
{
    return BtlOpenFxObj2(g_btl_fx_target, FX_TIMER);
}
