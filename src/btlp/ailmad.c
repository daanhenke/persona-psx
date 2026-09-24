/* Persona 1 (JP) - the turn a maddened fighter takes.  BTLP only.
 *   0x80095CB8 BtlAilmentTurnMad
 *
 * Entry 22 of g_btl_ailment_turn, and the only one that makes the fighter cast
 * rather than stopping it. A member under it turns on its own side: the move
 * comes off the Persona it is carrying and the targets are picked out of the
 * party.
 *
 * Four things have to hold first - the character's list is not shut, it has a
 * Persona in it, it can afford the Persona's cheapest move, and the Persona
 * gets on with the character well enough. A Persona and a character agree to
 * one of four degrees, two bits a character key in the record's +0x0C, and
 * anything under the third turns the whole turn into nothing.
 *
 * The move is one of the Persona's own, taken at random out of those in the
 * two ranges the routine will cast, and how it is aimed decides how it is
 * pointed:
 *
 *   - at one fighter: the slowest one left that the element reaches and that
 *     is not warded, the rest struck off the pickable list as they are turned
 *     down;
 *   - at the whole side: cast if fewer than two of the pickable fighters would
 *     turn it aside;
 *   - anything else: the same walk as the first, but the shape is laid over
 *     each candidate in turn and the cast only goes ahead when fewer than two
 *     of what it covers would turn it aside. The pickable list is copied aside
 *     before each try and put back after one that fails.
 *
 * The last of those reads the ward flags of the fighter one past the end of
 * the side rather than of the candidate it just weighed - the walk's counter
 * is left where the copy loop finished. It is the image's own reading and is
 * kept.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <rand.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/damage.h>
#include <persona/btlp/round.h>
#include <persona/btlp/stats.h>
#include <persona/btlp/status.h>
#include <persona/common/persona.h>
#include <persona/common/spell.h>

/* The two runs of move ids a maddened fighter will cast, each as the first id
   and how many follow it. */
#define MAD_FIRST_LOW   1
#define MAD_FIRST_RUN   0x36
#define MAD_SECOND_LOW  0x4D
#define MAD_SECOND_RUN  6

/* The two ways of aiming the routine knows by name; everything else is a
   shape laid over the field. */
#define MAD_AIM_ONE  0
#define MAD_AIM_SIDE 4

/* The flags that say the move does not reach: the fighter is warded. */
#define MAD_WARDED 0x1880

/* How many of what the move covers may turn it aside and still let it go. */
#define MAD_REFUSED 2

/* What the handler leaves in the byte. Nothing in BtlAilmentTakeTurn's switch
   answers to it, so the turn is left exactly as the handler aimed it. */
#define MAD_ACT 6

/* 97.98%: the spell count and the slot the aim lands on are one variable,
   n. The two arms that end up aiming share their tail, and the image
   shares it one statement earlier than this does - the slot plus five is
   worked out in each arm and lands in the shared block already in the return
   register, where this works it out at the label and has to carry the slot
   into a saved register to get it there. Passing the sum in a local of its own
   does put the two in step, and costs the arm above it the exit it shares with
   every other giving-up path. */
#ifdef NON_MATCHING
void BtlAilmentTurnMad(BtlActor *a, u_char *act)
{
    BtlStats  *p;
    SpellData *s;
    int        spell;
    u_char     list[BTL_STATS_SPELLS];
    int        i;
    int        n;
    int        refused;
    int        damage;

    p = &g_btl_personas[BtlActorPersona(a->obj->mark_num)];
    if (a->c.blocked != 0 || a->c.entry == CHAR_NO_ENTRY
        || a->c.sp < p->sp_cost
        || ((p->bond >> ((a->c.key - 1) * PERSONA_BOND_BITS))
            & PERSONA_BOND_MASK)
               < PERSONA_BOND_WILLING) {
        *act = AIL_ACT_NONE;
        return;
    }

    n = 0;
    for (i = 0; i < BTL_STATS_SPELLS; i++) {
        spell = p->spell[i];
        if ((u_int)(spell - MAD_FIRST_LOW) < MAD_FIRST_RUN
            || (u_int)(spell - MAD_SECOND_LOW) < MAD_SECOND_RUN) {
            list[n] = spell;
            n++;
        }
    }
    if (n == 0) {
        *act = AIL_ACT_NONE;
        return;
    }

    spell = list[rand() % n];
    s = &g_spell_data[spell];
    switch (s->target) {
    case MAD_AIM_ONE:
        BtlSetPickable();
        for (;;) {
            n = BtlSlowestOrder();
            if (n < 0) {
                *act = AIL_ACT_NONE;
                return;
            }
            damage = 0;
            if (BtlApplyAffinity(&damage, s->element,
                                 g_btl_combatants[n].c.resist) >= 0
                && (g_btl_combatants[n].flags & MAD_WARDED) == 0) {
                a->order   = n + BTL_PARTY;
                a->targets = 1 << (n + BTL_PARTY);
                break;
            }
            g_btl_combatants[n].pickable = 0;
        }
        break;

    case MAD_AIM_SIDE:
        BtlSetPickable();
        damage  = 0;
        refused = 0;
        for (i = 0; i < BTL_ENEMIES; i++) {
            if (g_btl_combatants[i].pickable != 0
                && (BtlApplyAffinity(&damage, s->element,
                                     g_btl_combatants[i].c.resist) < 0
                    || (g_btl_combatants[i].flags & MAD_WARDED) != 0)) {
                refused++;
            }
        }
        if (refused >= MAD_REFUSED) {
            *act = AIL_ACT_NONE;
            return;
        }
        n = BtlSlowestOrder();
    aim:
        a->order   = n + BTL_PARTY;
        a->targets = BtlPickableMask();
        break;

    default:
        BtlSetPickable();
        for (;;) {
            for (i = 0; i < BTL_ENEMIES; i++) {
                g_btl_combatants[i].pick_saved = g_btl_combatants[i].pickable;
            }
            n = BtlSlowestOrder();
            if (n < 0) {
                *act = AIL_ACT_NONE;
                return;
            }
            damage = 0;
            if (BtlApplyAffinity(&damage, s->element,
                                 g_btl_combatants[n].c.resist) >= 0
                && (g_btl_combatants[i].flags & MAD_WARDED) == 0) {
                BtlMarkEnemiesAround(&g_btl_combatants[n], s->target);
                refused = 0;
                for (i = 0; i < BTL_ENEMIES; i++) {
                    if (g_btl_combatants[i].pickable != 0
                        && (BtlApplyAffinity(&damage, s->element,
                                             g_btl_combatants[i].c.resist) < 0
                            || (g_btl_combatants[i].flags & MAD_WARDED) != 0)) {
                        refused++;
                    }
                }
                if (refused < MAD_REFUSED) {
                    goto aim;
                }
                for (i = 0; i < BTL_ENEMIES; i++) {
                    g_btl_combatants[i].pickable = g_btl_combatants[i].pick_saved;
                }
            }
            g_btl_combatants[n].pickable = 0;
        }
        break;
    }
    a->move = spell;
    *act = MAD_ACT;
}
#else
INCLUDE_ASM("btlp/nonmatchings/ailmad", BtlAilmentTurnMad);
#endif
