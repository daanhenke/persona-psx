/* Persona 1 (JP) - the two moves whose effect is three layers on the target.
 * BTLP only.
 *   0x800B6C7C BtlFxStartLayers
 *   0x800B6CA4 BtlFxStartLayers2
 *
 * Both are start handlers out of g_btl_spell_fx, and both are the same
 * routine: hand the fighter the move is aimed at to BtlOpenFxLayers and answer
 * the chain it builds. Which artwork the layers are drawn from is not decided
 * here - BtlStartMoveFx has already staged it - so two moves that differ only
 * in their pictures need two identical handlers, and the image has them.
 */
#include <decomp/types.h>
#include <persona/btlp/object.h>
#include <persona/btlp/spellfx.h>

BtlObj *BtlFxStartLayers(void)
{
    return BtlOpenFxLayers(g_btl_fx_target, FX_TIMER);
}

BtlObj *BtlFxStartLayers2(void)
{
    return BtlOpenFxLayers(g_btl_fx_target, FX_TIMER);
}
