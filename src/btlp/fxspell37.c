/* Persona 1 (JP) - two moves whose records sit ten apart in the table.
 * BTLP only.
 *   0x800BAA50 BtlFxStart37  0x800BAA7C BtlFxStart41
 *
 * Two start handlers out of g_btl_spell_fx standing next to each other in the
 * image although their moves are 0x37 and 0x41: the assembler laid them down
 * where they were written, and the table reaches both wherever they are.
 */
#include <decomp/types.h>
#include <persona/btlp/object.h>
#include <persona/btlp/spellfx.h>

/* None of these staggers its records. */
#define FX_TIMER 0

#define FX_WHITE 0xFF

BtlObj *BtlFxStart37(void)
{
    return BtlOpenFxOnTargets(FX_WHITE, FX_WHITE, FX_WHITE, FX_TIMER);
}

BtlObj *BtlFxStart41(void)
{
    return BtlOpenFxObj2(g_btl_fx_target, FX_TIMER);
}
