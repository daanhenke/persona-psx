/* Persona 1 (JP) - what being happy does to a fighter's turn.  BTLP only.
 *   0x800952A4 BtlAilmentTurnHappy
 *
 * Entry 1 of g_btl_ailment_turn. The fighter is too pleased with itself to
 * act, and how likely that is rises with how deep the ailment has been driven:
 * an even chance at the shallowest, three in four one step down, and always at
 * the deepest.
 *
 * The roll is taken before the level is looked at, which is why one call to
 * rand covers all three arms.
 */
#include <decomp/types.h>
#include <rand.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/status.h>

/* The roll is a byte, and the two levels that only sometimes hold the fighter
   test it against these. The compare is unsigned: gcc knows the masked roll
   cannot be negative and picks slti unless the cast says otherwise. */
#define HAPPY_ROLL   0xFF
#define HAPPY_HOLD_0 0x80
#define HAPPY_HOLD_1 0xC0

void BtlAilmentTurnHappy(BtlActor *a, u_char *act)
{
    int roll;

    roll = rand();
    switch ((signed char)a->c.ail_level) {
    case 0:
        if ((u_int)(roll & HAPPY_ROLL) < HAPPY_HOLD_0) {
            *act = 0;
        }
        break;
    case 1:
        if ((u_int)(roll & HAPPY_ROLL) < HAPPY_HOLD_1) {
            *act = 0;
        }
        break;
    default:
        *act = 0;
        break;
    }
}
