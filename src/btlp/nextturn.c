/* Persona 1 (JP) - getting a fighter's turn ready before it comes round.
 *   BTLP only.
 *   0x80092684 BtlReadyNextTurn   0x80092A14 BtlReadyTurnNow
 *
 * Both walk the round's order from where it stands, give the first fighter
 * whose turn is still to come its action, and start reading that fighter's
 * sound bank - a member's out of the battle's pack, an enemy's out of the voice
 * banks - putting the slot up on the debug HUD as they do. BtlReadyNextTurn
 * looks past the turn being played and reads in the background, so the bank is
 * in by the time the turn arrives; BtlReadyTurnNow starts at the turn itself
 * and waits, for a turn the round reached before its bank did.
 *
 * A fighter already has its action unless the action is TURN_UNSET. Under a
 * scripted placement the script aims it. Otherwise a member takes the action
 * its standing tactic gives - or, once a negotiation has ended, the one the
 * marker it left says - and an enemy picks a move, which gives the action
 * through the second table. The six special keys choose again every time.
 *
 * The six round flags behind the two tables are defined here only because
 * this is where the image keeps them: they start at an odd address straight
 * after the second table, so they cannot be a section of their own.
 */
#include <decomp/types.h>
#include <persona/common/persona.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/number.h>
#include <persona/btlp/pack.h>
#include <persona/btlp/round.h>
#include <persona/btlp/status.h>

/* An action nobody has chosen yet, which is also what a spent turn holds. */
#define TURN_UNSET 0xFF

/* The keys that always choose again. */
#define TURN_KEY_FIRST 0xAA
#define TURN_KEY_LAST  0xAF

/* A member under this flag makes action 4 whatever its tactic says. */
#define TURN_FORCED        0x8000000
#define TURN_FORCED_ACTION 4

/* An enemy whose pick gives this action has no move to make. */
#define TURN_ACTION_ATTACK 2

/* Flags under which a fighter's bank is not worth reading ahead. */
#define TURN_NO_BANK 0x803A000

#define TURN_TACTIC 0xF

/* The action a member's standing tactic gives it, by the tactic. */
u_char g_btl_tactic_actions[6] = {2, 6, 3, 0xC, 5, 4};

/* The action an enemy's pick gives it, by what BtlChooseEnemyMove answered:
   one of its six spells, or one of the four things it can do instead. */
u_char g_btl_enemy_pick_actions[11] = {2, 6, 6, 6, 6, 6, 6, 4, 0xE, 0, 5};

u_char g_btl_party_lost = 0;
u_char g_btl_boss22_shape = 0;
u_char g_btl_boss22_shown = 0;
u_char g_btl_boss20_shape = 0;
u_char g_btl_boss20_shown = 0;
u_char g_btl_round_over_done = 0;

void BtlReadyNextTurn(void)
{
    int       turn;
    int       slot;
    BtlActor *a;
    int       pick;
    int       tactic;

    if (g_btl_act_kind == 3) {
        return;
    }
    for (turn = (signed char)g_btl_turn + (g_btl_act_kind == 0);
         turn < g_btl_turns; turn++) {
        slot = g_btl_turn_order[turn];
        if ((g_btl_actors[slot].c.key >= TURN_KEY_FIRST
             && g_btl_actors[slot].c.key <= TURN_KEY_LAST)
            || g_btl_place_party != 0) {
            g_btl_actors[slot].action = TURN_UNSET;
        }
        if (g_btl_actors[slot].c.key == 0
            || (signed char)g_btl_actors[slot].c.status == BTL_STATUS_DOWN
            || (g_btl_actors[slot].flags & BTL_ACTOR_OUT) != 0
            || g_btl_actors[slot].action != TURN_UNSET) {
            continue;
        }
        if (slot < BTL_PARTY) {
            if (g_btl_place_party != 0) {
                BtlAimScriptedMember(&g_btl_actors[slot]);
            } else {
                if ((g_btl_actors[slot].flags & TURN_FORCED) != 0) {
                    g_btl_actors[slot].action = TURN_FORCED_ACTION;
                } else {
                    /* Assigned inside the subscript: a bare conditional
                       there is folded into both arms, and the table's
                       address is lifted into a register of its own. */
                    g_btl_actors[slot].action = g_btl_tactic_actions[tactic =
                        g_btl_talk_outcome != 0
                            ? g_btl_actors[slot].mark_kind
                            : g_btl_actors[slot].c.unk5D & TURN_TACTIC];
                }
                BtlAilmentTakeTurn(&g_btl_actors[slot],
                                   &g_btl_actors[slot].action);
            }
        } else {
            if (g_btl_place_party != 0) {
                BtlAimScriptedEnemy(&g_btl_actors[slot]);
            } else {
                a = &g_btl_actors[slot];
                pick = BtlChooseEnemyMove(a);
                g_btl_actors[slot].action = g_btl_enemy_pick_actions[pick];
                g_btl_actors[slot].move =
                    g_persona_data[g_btl_actors[slot].c.key].spell[pick - 1];
                g_btl_actors[slot].obj->unkD3 = pick - 1;
                if (g_btl_actors[slot].action == TURN_ACTION_ATTACK) {
                    g_btl_actors[slot].move = 0;
                    BtlAimMove(a);
                }
                BtlAilmentTakeTurn(a, &g_btl_actors[slot].action);
            }
        }
        if ((g_btl_actors[slot].flags & TURN_NO_BANK) == 0) {
            BtlDrawNumberAlt(g_btl_bank_slot_cells, slot, BTL_BANK_CELLS);
            if (slot < BTL_PARTY) {
                BtlReadMemberBank(0, g_btl_actors[slot].c.key);
            } else {
                BtlReadPackBank(0, g_btl_actors[slot].c.key);
            }
            return;
        }
    }
}

void BtlReadyTurnNow(void)
{
    int       turn;
    int       slot;
    BtlActor *a;
    int       pick;
    int       tactic;

    for (turn = (signed char)g_btl_turn; turn < g_btl_turns; turn++) {
        slot = g_btl_turn_order[turn];
        if ((g_btl_actors[slot].c.key >= TURN_KEY_FIRST
             && g_btl_actors[slot].c.key <= TURN_KEY_LAST)
            || g_btl_place_party != 0) {
            g_btl_actors[slot].action = TURN_UNSET;
        }
        if (g_btl_actors[slot].c.key == 0
            || (signed char)g_btl_actors[slot].c.status == BTL_STATUS_DOWN
            || (g_btl_actors[slot].flags & BTL_ACTOR_OUT) != 0
            || g_btl_actors[slot].action != TURN_UNSET) {
            continue;
        }
        if (slot < BTL_PARTY) {
            if (g_btl_place_party != 0) {
                BtlAimScriptedMember(&g_btl_actors[slot]);
            } else {
                if ((g_btl_actors[slot].flags & TURN_FORCED) != 0) {
                    g_btl_actors[slot].action = TURN_FORCED_ACTION;
                } else {
                    /* Assigned inside the subscript: a bare conditional
                       there is folded into both arms, and the table's
                       address is lifted into a register of its own. */
                    g_btl_actors[slot].action = g_btl_tactic_actions[tactic =
                        g_btl_talk_outcome != 0
                            ? g_btl_actors[slot].mark_kind
                            : g_btl_actors[slot].c.unk5D & TURN_TACTIC];
                }
                BtlAilmentTakeTurn(&g_btl_actors[slot],
                                   &g_btl_actors[slot].action);
            }
        } else {
            if (g_btl_place_party != 0) {
                BtlAimScriptedEnemy(&g_btl_actors[slot]);
            } else {
                a = &g_btl_actors[slot];
                pick = BtlChooseEnemyMove(a);
                g_btl_actors[slot].action = g_btl_enemy_pick_actions[pick];
                g_btl_actors[slot].move =
                    g_persona_data[g_btl_actors[slot].c.key].spell[pick - 1];
                g_btl_actors[slot].obj->unkD3 = pick - 1;
                if (g_btl_actors[slot].action == TURN_ACTION_ATTACK) {
                    g_btl_actors[slot].move = 0;
                    BtlAimMove(a);
                }
                BtlAilmentTakeTurn(a, &g_btl_actors[slot].action);
            }
        }
        if ((g_btl_actors[slot].flags & TURN_NO_BANK) == 0) {
            BtlDrawNumberAlt(g_btl_bank_slot_cells, slot, BTL_BANK_CELLS);
            if (slot < BTL_PARTY) {
                BtlReadMemberBank(1, g_btl_actors[slot].c.key);
            } else {
                BtlReadPackBank(1, g_btl_actors[slot].c.key);
            }
            return;
        }
    }
}
