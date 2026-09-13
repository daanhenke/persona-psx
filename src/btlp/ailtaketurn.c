/* Persona 1 (JP) - handing a fighter's turn to its ailment.  BTLP only.
 *   0x800951D0 BtlAilmentTakeTurn
 *
 * Runs the handler g_btl_ailment_turn keeps for the fighter's ailment - there
 * is none for a healthy one - and then marks the fighter for the answer the
 * handler left in the byte. A lost turn sets one bit and a turn it went
 * nowhere with sets the flinch; a redirected or fleeing turn clears the lot,
 * the fleeing one leaving 0x10000 alone. Whatever the answer, the battle
 * message is held for ninety frames.
 */
#include <decomp/types.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/round.h>
#include <persona/btlp/status.h>

/* What each answer sets or clears on the fighter's flags. */
#define TURN_LOST_BIT   0x20000
#define TURN_AIMED_BITS (0x08000000 | 0x30000 | 0x8000 | BTL_ACTOR_FLINCHED)
#define TURN_FLEE_BITS  (0x08000000 | 0x20000 | 0x8000 | BTL_ACTOR_FLINCHED)

#define TURN_MSG_HOLD 90

void BtlAilmentTakeTurn(BtlActor *a, u_char *act)
{
    void (*turn)();

    turn = g_btl_ailment_turn[(signed char)a->c.status];
    if (turn != 0) {
        turn(a, act);
    }
    switch (*act) {
    case AIL_ACT_AIMED:
    case AIL_ACT_GUN:
        a->flags &= ~TURN_AIMED_BITS;
        break;
    case AIL_ACT_FLEE:
        a->flags &= ~TURN_FLEE_BITS;
        break;
    case AIL_ACT_NONE:
        a->flags |= BTL_ACTOR_FLINCHED;
        break;
    case AIL_ACT_LOST:
        a->flags |= TURN_LOST_BIT;
        break;
    }
    g_btl_msg_timer = TURN_MSG_HOLD;
}
