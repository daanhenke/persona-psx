/* Persona 1 (JP) - the move whose effect is built somewhere else.  BTLP only.
 *   0x800B9C08 BtlFxStart31
 *
 * A start handler out of g_btl_spell_fx which hands the fighter it is aimed at
 * to the stack opener further down the block and answers what that builds. The target
 * is loaded before the stack is opened, which is gcc putting a leaf's argument
 * above the prologue and is why the boundary finder gave those two instructions
 * to the routine in front of this one.
 */
#include <decomp/types.h>
#include <persona/btlp/object.h>
#include <persona/btlp/spellfx.h>

BtlObj *BtlFxStart31(void)
{
    return BtlOpenFxStack(g_btl_fx_target);
}
