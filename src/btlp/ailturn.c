/* Persona 1 (JP) - seventeen of the ailments and what they do to a turn.
 * BTLP only.
 *   0x80095870 BtlAilmentTurnCharm   0x80095930 BtlAilmentTurnFreeze
 *   0x80095938 BtlAilmentTurnShock   0x80095940 BtlAilmentTurnBind
 *   0x80095948 BtlAilmentTurnSleep   0x80095950 BtlAilmentTurnClose
 *   0x80095958 BtlAilmentTurnBlind   0x80095960 BtlAilmentTurnUnluck
 *   0x80095968 BtlAilmentTurnTerror  0x80095A40 BtlAilmentTurnGuilt
 *   0x80095A5C BtlAilmentTurnPoison  0x80095A64 BtlAilmentTurnParalyse
 *   0x80095A6C BtlAilmentTurnStone   0x80095A74 BtlAilmentTurnSick
 *   0x80095A7C BtlAilmentTurnDead    0x80095A84 BtlAilmentTurnCloak
 *   0x80095A8C BtlAilmentTurnPuppet
 *
 * Entries 3 to 19 of g_btl_ailment_turn, in the order the table has them.
 * Each is handed the fighter and the byte its turn will be run from, and
 * leaves that byte alone unless the ailment has something to say: nought
 * cancels the turn and anything else redirects it.
 *
 * Most of them are two instructions. Freeze, shock, bind, sleep, paralysis and
 * petrification simply cancel; close, blindness, ill luck, poison, sickness,
 * death, the cloak and the puppet string do not touch the turn at all, and are
 * here as entries rather than as gaps because the table is indexed straight by
 * the ailment code. That is also why they had no names: nothing calls one, so
 * the boundary finder gave all seven of the run behind the guilt handler to
 * the guilt handler.
 *
 * The two that do real work are charm and terror. A charmed fighter turns on
 * its own side - the record's own bit says which side it is on, and the pick
 * comes back negative when there is nobody left to turn on. A terrified one
 * runs, and only if the fight is one that may be run from at all and the
 * enemy is not ten or more levels above the party; how likely that is rises
 * with the level, a quarter of the time at the shallowest and three quarters
 * at the deepest.
 */
#include <decomp/types.h>
#include <rand.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/object.h>
#include <persona/btlp/round.h>
#include <persona/btlp/sides.h>
#include <persona/btlp/status.h>

/* The attribute bit that says the record belongs to the other side, which is
   the whole of what picks between the two pickers. */
#define AIL_OTHER_SIDE 0x200

/* Charm clears these two off a fighter it turns round. */
#define AIL_CHARM_CLEAR 0x18000

/* How far above the party an enemy has to be before terror stops working. */
#define AIL_TERROR_GAP 10

/* The rolls the three levels of terror take, and which way each is read. */
#define AIL_TERROR_ROLL_0 3
#define AIL_TERROR_ROLL_1 1
#define AIL_TERROR_ROLL_2 3

void BtlAilmentTurnCharm(BtlActor *a, u_char *act)
{
    BtlObj *o;
    int     slot;

    o = a->obj;
    if ((o->attr & AIL_OTHER_SIDE) == 0) {
        slot = BtlPickOtherMember(o->mark_num);
        if (slot >= 0) {
            a->targets = 1 << slot;
            a->order = slot;
            *act = AIL_ACT_AIMED;
            return;
        }
    } else {
        slot = BtlPickOtherEnemy(o->mark_num);
        if (slot >= 0) {
            a->targets = 1 << slot;
            a->order = slot;
            a->flags &= ~AIL_CHARM_CLEAR;
            *act = AIL_ACT_AIMED;
            return;
        }
    }
    *act = 0;
}

void BtlAilmentTurnFreeze(BtlActor *a, u_char *act)
{
    *act = 0;
}

void BtlAilmentTurnShock(BtlActor *a, u_char *act)
{
    *act = 0;
}

void BtlAilmentTurnBind(BtlActor *a, u_char *act)
{
    *act = 0;
}

void BtlAilmentTurnSleep(BtlActor *a, u_char *act)
{
    *act = 0;
}

void BtlAilmentTurnClose(BtlActor *a, u_char *act)
{
}

void BtlAilmentTurnBlind(BtlActor *a, u_char *act)
{
}

void BtlAilmentTurnUnluck(BtlActor *a, u_char *act)
{
}

void BtlAilmentTurnTerror(BtlActor *a, u_char *act)
{
    if (g_btl_enemy_level - g_btl_party_level < AIL_TERROR_GAP
        && g_btl_no_escape == 0) {
        switch ((signed char)a->c.ail_level) {
        case 0:
            if ((rand() & AIL_TERROR_ROLL_0) == 0) {
                *act = AIL_ACT_FLEE;
            }
            break;
        case 1:
            if ((rand() & AIL_TERROR_ROLL_1) != 0) {
                *act = AIL_ACT_FLEE;
            }
            break;
        case 2:
            if ((rand() & AIL_TERROR_ROLL_2) != 0) {
                *act = AIL_ACT_FLEE;
            }
            break;
        default:
            break;
        }
    }
}

void BtlAilmentTurnGuilt(BtlActor *a, u_char *act)
{
    if ((signed char)a->c.ail_level == BTL_AIL_DEEPEST) {
        *act = 0;
    }
}

void BtlAilmentTurnPoison(BtlActor *a, u_char *act)
{
}

void BtlAilmentTurnParalyse(BtlActor *a, u_char *act)
{
    *act = 0;
}

void BtlAilmentTurnStone(BtlActor *a, u_char *act)
{
    *act = 0;
}

void BtlAilmentTurnSick(BtlActor *a, u_char *act)
{
}

void BtlAilmentTurnDead(BtlActor *a, u_char *act)
{
}

void BtlAilmentTurnCloak(BtlActor *a, u_char *act)
{
}

void BtlAilmentTurnPuppet(BtlActor *a, u_char *act)
{
}
