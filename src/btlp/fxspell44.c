/* Persona 1 (JP) - two moves that borrow 0x47's effect, and one of their own.
 * BTLP only.
 *   0x800BBB20 BtlFxStart44  0x800BBB40 BtlFxStart45  0x800BBB60 BtlFxStart46
 *
 * Start handlers out of g_btl_spell_fx. The first two call move 0x47's handler,
 * which stands immediately behind them and which four more moves borrow as
 * well; the third opens a single record on the fighter aimed at.
 */
#include <decomp/types.h>
#include <persona/btlp/object.h>
#include <persona/btlp/spellfx.h>

/* None of these staggers its records. */
#define FX_TIMER 0

BtlObj *BtlFxStart44(void)
{
    return BtlFxStart47();
}

BtlObj *BtlFxStart45(void)
{
    return BtlFxStart47();
}

BtlObj *BtlFxStart46(void)
{
    return BtlOpenFxObj2(g_btl_fx_target, FX_TIMER);
}
