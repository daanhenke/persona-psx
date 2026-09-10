/* Persona 1 (JP) - what the party does once a negotiation is over.  BTLP only.
 *   0x8009BDC8 BtlTalkersStay        0x8009BEE8 BtlTalkersJoin
 *   0x8009C16C BtlTalkersLeaveField  0x8009C370 BtlReadyItemAction
 *   0x8009C54C BtlReadySpellAction
 *
 * A negotiation suspends the round: the member doing the talking is taken out
 * of the order and everybody else's chosen action is put aside. When it ends,
 * g_btl_talk_outcome says which of three ways the round is picked back up, and
 * BtlStageRound calls the matching one of the three below. Each walks the five
 * party slots, skipping anyone absent, down, out of the fight, or already
 * carrying a marker of their own, and gives the rest an action again:
 *
 *   1 stay        everyone swings with the weapon they are holding
 *   2 leave       everyone goes back to whatever they had chosen before
 *   3 join        everyone swings with the weapon their own setting picks,
 *                 the gun if they have one loaded and the melee weapon if not
 *
 * The last two of the five are the per-fighter half of that. Both take an
 * action that has already been chosen and make it ready for the round: they
 * ask BtlMarkMoveArea which cells it reaches, take a place in the order five
 * behind the slowest fighter, and write the target mask. Neither picks the
 * action - that is the menu's job - and both fail the same way, by putting up
 * marker 5 and setting the object's script to 4, which is the "cannot do that"
 * shake.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/round.h>
#include <persona/common/item.h>

/* The walk is the same in all three: g_btl_actor_turn is the counter as well
   as the slot, so the tests read the record through it while the pointer next
   to it carries the writes and the call. */
void BtlTalkersStay(void)
{
    BtlActor *a;

    a = g_btl_actors;
    g_btl_actor_turn = 0;
    do {
        if (g_btl_actors[g_btl_actor_turn].c.key != 0
            && (signed char)g_btl_actors[g_btl_actor_turn].c.status
                   != BTL_STATUS_DOWN
            && (g_btl_actors[g_btl_actor_turn].flags & BTL_ACTOR_OUT) == 0
            && g_btl_actors[g_btl_actor_turn].marker < 2) {
            a->marker = 1;
            BtlReadyItemAction(a, &g_item_defs[a->c.equip[0]]);
        }
        a++;
        g_btl_actor_turn++;
    } while (g_btl_actor_turn < BTL_PARTY);
    g_btl_talk_outcome = 1;
}

/* Everyone swings with whatever their own setting says they are holding, and
   the two arms only differ in which slot the weapon comes out of. The gun
   needs both its own entry and its ammunition, and a member who has one
   without the other is refused rather than falling back to the melee weapon. */
void BtlTalkersJoin(void)
{
    BtlActor *a;

    a = g_btl_actors;
    g_btl_actor_turn = 0;
    do {
        if (g_btl_actors[g_btl_actor_turn].c.key != 0
            && (signed char)g_btl_actors[g_btl_actor_turn].c.status
                   != BTL_STATUS_DOWN
            && (g_btl_actors[g_btl_actor_turn].flags & BTL_ACTOR_OUT) == 0
            && a->marker < 2) {
            switch (a->unkC8) {
            case 0:
                a->marker = 4;
                BtlReadyItemAction(a, &g_item_defs[a->c.equip[0]]);
                g_btl_actors[g_btl_actor_turn].c.unk5D = 0;
                g_btl_actors[g_btl_actor_turn].order_kept =
                    g_btl_actors[g_btl_actor_turn].order;
                g_btl_actors[g_btl_actor_turn].targets_kept =
                    g_btl_actors[g_btl_actor_turn].targets;
                break;
            case 1:
                if (a->c.equip[1] != 0 && a->c.equip[2] != 0) {
                    a->marker = 4;
                    BtlReadyItemAction(a, &g_item_defs[a->c.equip[1]]);
                    g_btl_actors[g_btl_actor_turn].c.unk5D = 0x33;
                    g_btl_actors[g_btl_actor_turn].order_kept =
                        g_btl_actors[g_btl_actor_turn].order;
                    g_btl_actors[g_btl_actor_turn].targets_kept =
                        g_btl_actors[g_btl_actor_turn].targets;
                } else {
                    a->marker = 4;
                    a->mark_kind = 5;
                    BtlShowMarker(g_btl_actor_turn, 1, 5);
                    a->obj->motion = 4;
                }
                break;
            }
        }
        a++;
        g_btl_actor_turn++;
    } while (g_btl_actor_turn < BTL_PARTY);
    g_btl_talk_outcome = 3;
}

/* Not matched: the same shape and the same instructions bar one. gcc lifts
   the 1 that puts the marker up into a saved register, because the switch and
   the BtlShowMarker call want it too, and the image writes it out afresh at
   each of the four places instead - so this carries one register more and the
   frame is four bytes wider. Everything else lines up.

   The same again: the kept copies go back over the live ones and the action
   is aimed afresh, because the negotiation moved everybody about and a target
   mask taken before it is no longer worth anything. Char.unk5D carries the
   kind in its low nibble and the one before it in the high one, so shifting
   the high nibble down is what makes "again" mean the round before this. */
#ifdef NON_MATCHING
void BtlTalkersLeaveField(void)
{
    BtlActor *a;

    a = g_btl_actors;
    g_btl_actor_turn = 0;
    do {
        if (g_btl_actors[g_btl_actor_turn].c.key != 0
            && (signed char)g_btl_actors[g_btl_actor_turn].c.status
                   != BTL_STATUS_DOWN
            && (g_btl_actors[g_btl_actor_turn].flags & BTL_ACTOR_OUT) == 0
            && g_btl_actors[g_btl_actor_turn].marker < 2) {
            a->c.unk5D = (a->c.unk5D & 0xF0) | (a->c.unk5D >> 4);
            a->marker = 1;
            a->ail_line = a->ail_line_kept;
            a->order = a->order_kept;
            a->move = a->move_kept;
            a->targets = a->targets_kept;
            switch (a->c.unk5D & 0xF) {
            case 0:
                BtlReadyItemAction(a, &g_item_defs[a->c.equip[0]]);
                break;
            case 1:
                BtlReadySpellAction(a);
                break;
            case 3:
                if (a->c.equip[1] != 0 && a->c.equip[2] != 0) {
                    BtlReadyItemAction(a, &g_item_defs[a->c.equip[1]]);
                    break;
                }
            default:
                a->mark_kind = 5;
                BtlShowMarker(g_btl_actor_turn, 1, 5);
                a->obj->motion = 4;
            }
        }
        a++;
        g_btl_actor_turn++;
    } while (g_btl_actor_turn < BTL_PARTY);
    g_btl_talk_outcome = 2;
}
#else
INCLUDE_ASM("btlp/nonmatchings/talkorders", BtlTalkersLeaveField);
#endif

INCLUDE_ASM("btlp/nonmatchings/talkorders", BtlReadyItemAction);

INCLUDE_ASM("btlp/nonmatchings/talkorders", BtlReadySpellAction);
