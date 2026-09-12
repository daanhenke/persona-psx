/* Persona 1 (JP) - one move whose effect stands on the fighter it is aimed at.
 * BTLP only.
 *   0x800BF4D0 BtlFxStartA1
 *
 * A start handler out of g_btl_spell_fx, and the plainest body the table has:
 * one record on the target, arriving at once. Twenty-six moves are drawn this
 * way and each has an entry point of its own.
 */
#include <decomp/types.h>
#include <persona/btlp/object.h>
#include <persona/btlp/spellfx.h>

BtlObj *BtlFxStartA1(void)
{
    return BtlOpenFxObj2(g_btl_fx_target, FX_TIMER);
}
