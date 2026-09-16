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

/* The marker a refused action puts up, and what the fighter's object is put
   through under it. */
#define MARK_REFUSED     5
#define MARK_ITEM        3
#define BTL_MOTION_SHAKE 4

/* The bit of the item's group field the marker's kind is taken from. */
#define ITEM_GROUP_MARK 0x10

/* How far behind the slowest fighter a readied action comes in. */
#define TURN_GAP 5

/* The aim nibble of ItemDef.swing, which is SpellData.aim's nibble again:
   one fighter, a whole side either way round, or the weakest. */
#define AIM_MASK  0xF
#define AIM_ONE   1
#define AIM_SIDE  2
#define AIM_SIDE2 4
#define AIM_ONE2  8

/* The nine slots the enemy side reaches over, where they start in a target
   mask, and a number no fighter's hp reaches. */
#define BTL_COMBATANTS  9
#define BTL_ENEMY_SLOT0 5
#define HP_HIGHEST      0x3E7

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
            switch (a->tactic) {
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

/* A weapon or a used item made ready. BtlMarkMoveArea says which cells it
   reaches and answers negative when it reaches none; a fighter carrying guilt
   is refused whatever the answer was. Both refusals are the shake.

   Past that the action takes its place five behind the slowest fighter and the
   aim nibble of the item's own swing byte says what it is pointed at: one
   fighter, which is the slot the order itself names; a whole side, which is
   every slot that is filled and pickable; or the weakest, which is the lowest
   hp among those - and that one moves the order onto the fighter it found. */
void BtlReadyItemAction(BtlActor *a, const ItemDef *item)
{
    BtlActor *o;
    int       order;
    int       i;
    int       low;

    if (BtlMarkMoveArea(a, item->area, item->swing) >= 0
        && (signed char)a->c.status != BTL_STATUS_GUILT) {
    /* The marker's kind goes through the same local the order does, which is
       what keeps the pair in one register. */
    order        = (item->unk06 & ITEM_GROUP_MARK) != 0 ? MARK_ITEM : 0;
    a->mark_kind = order;
    BtlShowMarker(g_btl_actor_turn, 1, order);
    order = BtlSlowestOrder() + TURN_GAP;
    switch (item->swing & AIM_MASK) {
    case AIM_ONE:
        a->order   = order;
        a->targets = 1 << order;
        break;
    case AIM_SIDE:
    case AIM_SIDE2:
        a->order   = order;
        a->targets = 0;
        o          = g_btl_combatants;
        for (i = 0; i < BTL_COMBATANTS; i++) {
            if (o[i].c.key != 0 && o[i].pickable != 0) {
                a->targets |= 1 << (i + BTL_ENEMY_SLOT0);
            }
        }
        break;
    case AIM_ONE2:
        /* The order the slowest fighter gave it is thrown away here: the
           weakest fighter's own slot takes its place. */
        i   = 0;
        low = HP_HIGHEST;
        o   = g_btl_combatants;
        for (; i < BTL_COMBATANTS; i++) {
            if (o[i].c.key != 0 && o[i].pickable != 0 && o[i].c.hp < low) {
                low   = o[i].c.hp;
                order = i;
            }
        }
        a->order   = order + BTL_ENEMY_SLOT0;
        a->targets = 1 << (order + BTL_ENEMY_SLOT0);
        break;
    }
    } else {
        a->mark_kind = MARK_REFUSED;
        BtlShowMarker(g_btl_actor_turn, 1, MARK_REFUSED);
        a->obj->motion = BTL_MOTION_SHAKE;
    }
}

INCLUDE_ASM("btlp/nonmatchings/talkorders", BtlReadySpellAction);
