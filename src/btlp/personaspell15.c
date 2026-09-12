/* Persona 1 (JP) - spell 0x15 played out by a summoned Persona.  BTLP only.
 *   0x800B1D08 BtlPersonaSpell15
 *
 * One arm of the switch in the routine the persona attack motion hands to,
 * and the only one big enough to stand on its own. It works out what the
 * summon's move does to the fighter it is aimed at - the acting record comes
 * from g_btl_actor_turn and the target from what the aim step left behind -
 * and reaches the spell's own row of g_spell_data for the numbers.
 *
 * It is arithmetic on doubles, which is what makes it the longest of the arms:
 * the soft-float calls are most of its length.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>

INCLUDE_ASM("btlp/nonmatchings/personaspell15", BtlPersonaSpell15);
