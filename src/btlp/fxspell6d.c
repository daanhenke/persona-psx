/* Persona 1 (JP) - one move whose effect stands on the fighter it is aimed at.
 * BTLP only.
 *   0x800BCF44 BtlFxStart6D
 *
 * A start handler out of g_btl_spell_fx, and the plainest body the table has:
 * one record on the target, arriving at once. Twenty-six moves are drawn this
 * way and each has an entry point of its own.
 */
#include <decomp/types.h>
#include <persona/btlp/object.h>
#include <persona/btlp/spellfx.h>

/* None of these staggers its records. */
#define FX_TIMER 0

BtlObj *BtlFxStart6D(void)
{
    return BtlOpenFxObj2(g_btl_fx_target, FX_TIMER);
}
