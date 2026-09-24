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
#include <persona/common/persona.h>
#include <persona/common/spell.h>
#include <persona/btlp/stats.h>
#include <persona/btlp/pick.h>
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

/* The marker a readied cast puts up. */
#define MARK_SPELL 1



/* The deepest an ailment goes. */
#define BTL_AIL_LAST 2

/* How many free spell ids there are past SPELL_FREE_FIRST. */
#define SPELL_FREE_COUNT 0x17

/* Spell ids the cast is turned down for outright: one on its own and a pair
   the swing arm takes instead. */
#define SPELL_NEVER       0x42
#define SPELL_SWUNG_FIRST 0x6B
#define SPELL_SWUNG_LAST  0x6D

/* Which ids are spells: the free ones, and everything from here up bar the
   one that is swung. */
#define MOVE_SPELL_FIRST 0xA3
#define MOVE_SWUNG_SPELL 0xDB

/* The aim nibble of SpellData.aim, and what a move can reach. */
#define AIM_MASK   0xF
#define AIM_ONE    1
#define AIM_SIDE   2
#define AIM_SIDE2  4
#define AIM_ONE2   8
#define REACH_SIDE 4

/* The kinds that reach across the fight, and the plain swings inside them. */
#define KIND_14 0x14
#define KIND_1A 0x1A
#define KIND_1C 0x1C
#define KIND_1E 0x1E
#define KIND_32 0x32
#define MOVE_PLAIN_FIRST 0x53
#define MOVE_PLAIN_COUNT 3
#define MOVE_PLAIN_KIND  0x6E

/* The move byte read again where an ordinary read would be folded into the
   one before it, as aimmove.c has it: the image reads it fresh for each of
   the three range tests below, and reading it this way is also what keeps the
   comparisons signed rather than narrowed to the byte. */
#define BTL_MOVE(a) (*(volatile u_char *)&(a)->move)

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

/* 92.36%, from 85.84%. The cases are written in the image's order: 0, then
   3 (which goes to the refusal when either slot is empty), then 1, then the
   refusal. That lets 0 and 3 share the item call. Left: gcc lifts the 1 that
   puts the marker up into a saved register and the image writes it out
   afresh. The loop dump shows why: the marker's 1 is materialised in SImode
   and matches the case-1 compare the switch emits at its end, so the pair
   has savings 2 at life 2. Moving the marker store, a local for the kind, or
   a byte-wide switch value leaves the match in place.

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
            case 3:
                if (a->c.equip[1] != 0 && a->c.equip[2] != 0) {
                    BtlReadyItemAction(a, &g_item_defs[a->c.equip[1]]);
                    break;
                }
                goto refuse;
            case 1:
                BtlReadySpellAction(a);
                break;
            default:
            refuse:
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

/* A cast made ready. Far more can turn it down than an item can: the Persona
   has to still carry the spell, the member has to be holding a Persona at all
   and not be blocked, the SP has to be there, the bond has to be at least two,
   and the ailment has to allow it - guilt at any level, and a closed or blinded
   fighter at the last. A handful of ids are turned down outright.

   What is left is aimed the way BtlAimMove aims it: the free spells and the
   ids from 0xA3 up go through the record's aim nibble, everything else through
   its kind. The nibble takes the place in the order and points the action at
   one fighter or at everything the pickable mask covers; the kind's own arm
   puts the whole enemy side on and either takes the slowest fighter's place,
   picks another enemy, or marks the enemies around the one it settled on. */
/* The refusal is written out once, at the end of the kind arm, and every
   way out of the tests above jumps to it. The marker call at the very end is
   what the rest fall into. The kind arm's first read of the move byte is the
   volatile one, so the second test reads it again, as the image does. The
   single-target arm works its order out in i, which puts it in the image's
   register. */
void BtlReadySpellAction(BtlActor *a)
{
    const SpellData *spell;
    const BtlStats  *p;
    int              status;
    short            move;
    int              i;
    int              kind;

    spell = &g_spell_data[a->move];
    p     = &g_btl_personas[BtlActorPersona(g_btl_actor_turn)];
    for (i = 0; i < BTL_STATS_SPELLS; i++) {
        if (p->spell[i] == a->move) {
            break;
        }
    }
    if (i >= BTL_STATS_SPELLS
        || a->move == 0
        || a->c.blocked != 0
        || a->c.entry == CHAR_NO_ENTRY
        || a->c.sp < p->sp_cost
        || ((p->bond >> ((a->c.key - 1) * PERSONA_BOND_BITS))
            & PERSONA_BOND_MASK) < PERSONA_BOND_WILLING) {
        goto refuse;
    }
    status = a->c.status;
    if ((u_int)(status - BTL_STATUS_LIFTED) < 2) {
        goto refuse;
    }
    if ((signed char)status == BTL_STATUS_GUILT
        && (signed char)a->c.ail_level > 0) {
        goto refuse;
    }
    if ((u_int)(status - BTL_STATUS_CLOSE) < 2
        && (signed char)a->c.ail_level == BTL_AIL_LAST) {
        goto refuse;
    }
    /* Through a short of its own: read straight off the record the three
       tests are narrowed to the byte and come out unsigned. */
    move = a->move;
    if (move == SPELL_NEVER) {
        goto refuse;
    }
    if (move >= SPELL_NEVER) {
        if (move < SPELL_SWUNG_LAST) {
            if (move >= SPELL_SWUNG_FIRST) {
                goto refuse;
            }
        }
    }

    if ((u_int)(BTL_MOVE(a) - SPELL_FREE_FIRST) < SPELL_FREE_COUNT
        || ((u_int)a->move >= MOVE_SPELL_FIRST
            && a->move != MOVE_SWUNG_SPELL)) {
        kind = spell->aim & AIM_MASK;
        switch (kind) {
        case AIM_ONE:
        case AIM_ONE2:
            if (BtlMarkMoveArea(a, spell->target, kind) < 0) {
                goto refuse;
            }
            i            = BtlSlowestOrder() + TURN_GAP;
            a->mark_kind = MARK_SPELL;
            a->order     = i;
            a->targets   = 1 << i;
            BtlShowMarker(g_btl_actor_turn, 1, MARK_SPELL);
            return;
        case AIM_SIDE:
        case AIM_SIDE2:
            if (BtlMarkMoveArea(a, spell->target, kind) < 0) {
                goto refuse;
            }
            a->order     = BtlSlowestOrder() + TURN_GAP;
            a->targets   = BtlPickableMask();
            a->mark_kind = MARK_SPELL;
            BtlShowMarker(g_btl_actor_turn, 1, MARK_SPELL);
            return;
        }
        return;
    }

    switch (spell->kind & SPELL_KIND_MASK) {
    case KIND_14:
    case KIND_1A:
    case KIND_1C:
    case KIND_1E:
    case KIND_32:
        if ((u_int)(BTL_MOVE(a) - MOVE_PLAIN_FIRST) >= MOVE_PLAIN_COUNT
            && a->move != MOVE_PLAIN_KIND) {
            if (spell->target != 0) {
                a->mark_kind = MARK_SPELL;
                BtlShowMarker(g_btl_actor_turn, 1, MARK_SPELL);
                return;
            }
            if (g_btl_actors[a->order].c.key == 0) {
                goto refuse;
            }
            if ((signed char)g_btl_actors[a->order].c.status
                == BTL_STATUS_DOWN) {
                goto refuse;
            }
            if ((g_btl_actors[a->order].flags & BTL_ACTOR_OUT) == 0) {
                a->mark_kind = MARK_SPELL;
                BtlShowMarker(g_btl_actor_turn, 1, MARK_SPELL);
                return;
            }
        refuse:
            a->mark_kind = MARK_REFUSED;
            BtlShowMarker(g_btl_actor_turn, 1, MARK_REFUSED);
            a->obj->motion = BTL_MOTION_SHAKE;
            return;
        }
        break;
    }

    switch (spell->target) {
    case 0:
        BtlSetPickable();
        g_btl_enemy_slot = BtlSlowestOrder();
        a->order   = (u_char)g_btl_enemy_slot + BTL_ENEMY_SLOT0;
        a->targets = 1 << (g_btl_enemy_slot + BTL_ENEMY_SLOT0);
        break;
    case REACH_SIDE:
        if (a->move == MOVE_SWUNG_SPELL) {
            i = BtlPickOtherEnemy(0);
            if (i < 0) {
                a->order   = BtlSlowestOrder() + TURN_GAP;
                a->targets = 1 << a->order;
            } else {
                a->order   = i;
                a->targets = 1 << i;
            }
        } else {
            BtlSetPickable();
            a->order   = BtlSlowestOrder() + TURN_GAP;
            a->targets = BtlPickableMask();
        }
        break;
    default:
        BtlSetPickable();
        g_btl_enemy_slot = BtlSlowestOrder();
        BtlMarkEnemiesAround(&g_btl_combatants[g_btl_enemy_slot],
                             spell->target);
        a->order   = (u_char)g_btl_enemy_slot + BTL_ENEMY_SLOT0;
        a->targets = BtlPickableMask();
        break;
    }
    a->mark_kind = MARK_SPELL;
    BtlShowMarker(g_btl_actor_turn, 1, MARK_SPELL);
}
