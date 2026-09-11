/* Persona 1 (JP) - one record on the target, and one over everything in grey.
 * BTLP only.
 *   0x800BC6B4 BtlFxStart50  0x800BC6DC BtlFxStart51
 *
 * Two start handlers out of g_btl_spell_fx.
 */
#include <decomp/types.h>
#include <persona/btlp/object.h>
#include <persona/btlp/spellfx.h>

/* None of these staggers its records. */
#define FX_TIMER 0

#define FX_GREY 0x80

BtlObj *BtlFxStart50(void)
{
    return BtlOpenFxObj2(g_btl_fx_target, FX_TIMER);
}

BtlObj *BtlFxStart51(void)
{
    return BtlOpenFxOnTargets(FX_GREY, FX_GREY, FX_GREY, FX_TIMER);
}
