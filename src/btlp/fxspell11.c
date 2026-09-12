/* Persona 1 (JP) - three moves that shake the screen as they land.  BTLP only.
 *   0x800B8148 BtlFxStart11  0x800B8168 BtlFxStart13  0x800B819C BtlFxStart14
 *
 * Start handlers out of g_btl_spell_fx. The first borrows move 0x10's handler
 * outright; the other two put a single record on the fighter aimed at and raise
 * the shake flag first, which is what the camera reads to start knocking the
 * field about. The flag is set before the record is asked for, not after, so
 * the shake and the effect arrive on the same frame.
 */
#include <decomp/types.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/object.h>
#include <persona/btlp/spellfx.h>

BtlObj *BtlFxStart11(void)
{
    return BtlFxStart10();
}

BtlObj *BtlFxStart13(void)
{
    g_btl_shake_on = 1;
    return BtlOpenFxObj2(g_btl_fx_target, FX_TIMER);
}

BtlObj *BtlFxStart14(void)
{
    g_btl_shake_on = 1;
    return BtlOpenFxObj2(g_btl_fx_target, FX_TIMER);
}
