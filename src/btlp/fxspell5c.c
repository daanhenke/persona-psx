/* Persona 1 (JP) - fifteen moves whose effect is one of the two plainest.
 * BTLP only.
 *   0x800BCBF4 BtlFxStart5C  0x800BCC20 BtlFxStart5D  0x800BCC4C BtlFxStart5E
 *   0x800BCC78 BtlFxStart5F  0x800BCCA0 BtlFxStart60  0x800BCCC8 BtlFxStart61
 *   0x800BCCF0 BtlFxStart62  0x800BCD1C BtlFxStart63  0x800BCD48 BtlFxStart64
 *   0x800BCD74 BtlFxStart65  0x800BCD9C BtlFxStart66  0x800BCDC8 BtlFxStart67
 *   0x800BCDF0 BtlFxStart68  0x800BCE18 BtlFxStart69  0x800BCE40 BtlFxStart6A
 *
 * Fifteen start handlers out of g_btl_spell_fx, one per move, and between them
 * only two bodies: a record on everything the move reaches with the fighters
 * under it walked to white, or a single record on the one fighter it is aimed
 * at. Which artwork is drawn is not decided here - BtlStartMoveFx has already
 * staged it - so moves that differ only in their pictures need one handler
 * each with nothing to tell them apart, and the image has fifteen of them.
 *
 * The table is what reaches these, so the move's id is all there is to call
 * one by; they are numbered for it the way the fixed boards are.
 */
#include <decomp/types.h>
#include <persona/btlp/object.h>
#include <persona/btlp/spellfx.h>

BtlObj *BtlFxStart5C(void)
{
    return BtlOpenFxOnTargets(FX_WHITE, FX_WHITE, FX_WHITE, FX_TIMER);
}

BtlObj *BtlFxStart5D(void)
{
    return BtlOpenFxOnTargets(FX_WHITE, FX_WHITE, FX_WHITE, FX_TIMER);
}

BtlObj *BtlFxStart5E(void)
{
    return BtlOpenFxOnTargets(FX_WHITE, FX_WHITE, FX_WHITE, FX_TIMER);
}

BtlObj *BtlFxStart5F(void)
{
    return BtlOpenFxObj2(g_btl_fx_target, FX_TIMER);
}

BtlObj *BtlFxStart60(void)
{
    return BtlOpenFxObj2(g_btl_fx_target, FX_TIMER);
}

BtlObj *BtlFxStart61(void)
{
    return BtlOpenFxObj2(g_btl_fx_target, FX_TIMER);
}

BtlObj *BtlFxStart62(void)
{
    return BtlOpenFxOnTargets(FX_WHITE, FX_WHITE, FX_WHITE, FX_TIMER);
}

BtlObj *BtlFxStart63(void)
{
    return BtlOpenFxOnTargets(FX_WHITE, FX_WHITE, FX_WHITE, FX_TIMER);
}

BtlObj *BtlFxStart64(void)
{
    return BtlOpenFxOnTargets(FX_WHITE, FX_WHITE, FX_WHITE, FX_TIMER);
}

BtlObj *BtlFxStart65(void)
{
    return BtlOpenFxObj2(g_btl_fx_target, FX_TIMER);
}

BtlObj *BtlFxStart66(void)
{
    return BtlOpenFxOnTargets(FX_WHITE, FX_WHITE, FX_WHITE, FX_TIMER);
}

BtlObj *BtlFxStart67(void)
{
    return BtlOpenFxObj2(g_btl_fx_target, FX_TIMER);
}

BtlObj *BtlFxStart68(void)
{
    return BtlOpenFxObj2(g_btl_fx_target, FX_TIMER);
}

BtlObj *BtlFxStart69(void)
{
    return BtlOpenFxObj2(g_btl_fx_target, FX_TIMER);
}

BtlObj *BtlFxStart6A(void)
{
    return BtlOpenFxObj2(g_btl_fx_target, FX_TIMER);
}
