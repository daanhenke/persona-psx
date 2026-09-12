/* Persona 1 (JP) - three more moves that shake the screen.  BTLP only.
 *   0x800B8370 BtlFxStart16  0x800B83A8 BtlFxStart17  0x800B83E0 BtlFxStart18
 *
 * Start handlers out of g_btl_spell_fx, all three raising the shake flag. The
 * first two put a record over everything the move reaches; the third walks the
 * acting fighter's own targets to white itself and then opens a grid over the
 * whole field rather than records on the fighters, which is the only handler in
 * the table that does both.
 */
#include <decomp/types.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/object.h>
#include <persona/btlp/round.h>
#include <persona/btlp/spellfx.h>

BtlObj *BtlFxStart16(void)
{
    g_btl_shake_on = 1;
    return BtlOpenFxOnTargets(FX_WHITE, FX_WHITE, FX_WHITE, FX_TIMER);
}

BtlObj *BtlFxStart17(void)
{
    g_btl_shake_on = 1;
    return BtlOpenFxOnTargets(FX_WHITE, FX_WHITE, FX_WHITE, FX_TIMER);
}

BtlObj *BtlFxStart18(void)
{
    g_btl_shake_on = 1;
    BtlTintTargets(g_btl_actor_turn, FX_WHITE, FX_WHITE, FX_WHITE);
    return BtlOpenFxGrid(FX_GRID_TABLE);
}
