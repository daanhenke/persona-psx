/* Persona 1 (JP) - one record on the target, and one over everything.
 * BTLP only.
 *   0x800BA258 BtlFxStart34  0x800BA280 BtlFxStart35
 *
 * Two start handlers out of g_btl_spell_fx, the two plainest shapes one after
 * the other.
 */
#include <decomp/types.h>
#include <persona/btlp/object.h>
#include <persona/btlp/spellfx.h>

/* None of these staggers its records. */
#define FX_TIMER 0

#define FX_WHITE 0xFF

BtlObj *BtlFxStart34(void)
{
    return BtlOpenFxObj2(g_btl_fx_target, FX_TIMER);
}

BtlObj *BtlFxStart35(void)
{
    return BtlOpenFxOnTargets(FX_WHITE, FX_WHITE, FX_WHITE, FX_TIMER);
}
