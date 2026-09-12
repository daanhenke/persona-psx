/* Persona 1 (JP) - the two fighter editors on the debug page.  BTLP only.
 *   0x800A065C BtlDebugMemberAilment  0x800A09F0 BtlDebugEnemyAilment
 *
 * Rows 2 and 3 of g_btl_debug_actions, and the same routine twice over: one
 * walks the party with BtlPickMember, the other the field with BtlPickEnemy,
 * and both switch every slot on their side pickable first so nothing is
 * greyed out.
 *
 * Once a fighter is chosen the page edits two bytes of its record - the
 * ailment code and the level it is at - and shows the answer on the fighter
 * itself: the marker object is taken back out of hiding, given the script for
 * the new code out of g_btl_actor_gfx, and its attached piece given the level
 * mark, with the code plus 0x40 written into the piece's own index. Up steps
 * the level round its three values and carries into the code, which wraps at
 * the twenty-fourth; the other keys step the code on its own.
 *
 * Both answer zero, the way every handler on that page does, so the round
 * carries on as though nothing had been ordered.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>

INCLUDE_ASM("btlp/nonmatchings/debugailment", BtlDebugMemberAilment);

INCLUDE_ASM("btlp/nonmatchings/debugailment", BtlDebugEnemyAilment);
