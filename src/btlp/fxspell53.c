/* Persona 1 (JP) - three moves whose effect covers everything they reach.
 * BTLP only.
 *   0x800BC7AC BtlFxStart53  0x800BC7D8 BtlFxStart54  0x800BC804 BtlFxStart55
 *
 * Three start handlers out of g_btl_spell_fx and one body between them.
 */
#include <decomp/types.h>
#include <persona/btlp/object.h>
#include <persona/btlp/spellfx.h>

BtlObj *BtlFxStart53(void)
{
    return BtlOpenFxOnTargets(FX_WHITE, FX_WHITE, FX_WHITE, FX_TIMER);
}

BtlObj *BtlFxStart54(void)
{
    return BtlOpenFxOnTargets(FX_WHITE, FX_WHITE, FX_WHITE, FX_TIMER);
}

BtlObj *BtlFxStart55(void)
{
    return BtlOpenFxOnTargets(FX_WHITE, FX_WHITE, FX_WHITE, FX_TIMER);
}
