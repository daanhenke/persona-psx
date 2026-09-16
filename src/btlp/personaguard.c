/* Persona 1 (JP) - a Persona stepping out in front of its owner.  BTLP only.
 *   0x8009A1A8 BtlPersonaGuard
 *
 * The round calls this once it has aimed an enemy's action, and it walks the
 * party half of that action's target mask. A member whose own Persona answers
 * for it may have it come out and stand there: BtlFxResolve drops the hit for
 * whoever g_btl_effect_actor names, so the action lands on nobody, and the
 * round sends the Persona away again at the end of the turn.
 *
 * Three things have to be true of the Persona before it will: the bond with
 * this member at PERSONA_BOND_FULL, every one of its spell slots learned, and
 * BtlStats.unk41 - the byte that says what it does of its own accord - with
 * GUARD_TYPE in its high nibble. deathmotion.c reads the same byte for the
 * other thing a Persona does unasked, taking a fallen owner's turn, and looks
 * for 0x40 there.
 *
 * The low nibble picks which moves it answers, which is the switch below: the
 * plain swing alone, a costed spell, the demon spells, the free spells, or
 * anything at all. The two arms that read the move's own record leave the
 * support kinds out, so nothing comes out to guard against a heal.
 *
 * The odds are how badly the owner is hurt: one in two at a sixteenth of its
 * hp, one in four at an eighth, one in eight at a quarter.
 *
 * @bug The odds are worked out inside the walk but held outside it, so a
 * member above a quarter of its hp does not roll at nothing - it rolls on
 * whatever the last member the chain did answer for left behind.
 */
#include <decomp/types.h>
#include <rand.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/cast.h>
#include <persona/btlp/object.h>
#include <persona/btlp/round.h>
#include <persona/btlp/stats.h>
#include <persona/btlp/text.h>
#include <persona/common/persona.h>
#include <persona/common/spell.h>

/* The two fights no Persona guards in. */
#define GUARD_ENCOUNTER_A 5
#define GUARD_ENCOUNTER_B 0xF

/* The half of BtlStats.unk41 that says a Persona guards at all, and the five
   kinds its other half comes in: the plain swing, a costed spell, the demon
   spells, the free spells, and everything that does harm. */
#define GUARD_MASK  0xF0
#define GUARD_TYPE  0x20
#define GUARD_SWING 0x20
#define GUARD_SPELL 0x21
#define GUARD_DEMON 0x22
#define GUARD_FREE  0x23
#define GUARD_ANY   0x24

/* A Persona only guards once it has learned this many of its spells. */
#define GUARD_SLOTS 6

/* The move ids each kind covers. The demon spells are a run of their own;
   the free ones are SPELL_FREE_FIRST's run and a second above it; and
   GUARD_ANY reads the record below GUARD_HIGH_FIRST and takes the ids up to
   GUARD_HIGH_END on trust. */
#define GUARD_DEMON_FIRST 0xA3
#define GUARD_DEMON_COUNT 0x3C
#define GUARD_FREE_COUNT  0x17
#define GUARD_HIGH_FIRST  0x90
#define GUARD_HIGH_COUNT  0x10
#define GUARD_HIGH_END    0xDF

/* The move kinds that do no harm, which no Persona comes out against. */
#define GUARD_KIND_14 0x14
#define GUARD_KIND_1A 0x1A
#define GUARD_KIND_1C 0x1C
#define GUARD_KIND_1E 0x1E
#define GUARD_KIND_32 0x32

/* One in this many, by how far the owner's hp has fallen. */
#define GUARD_ODDS_16TH 2
#define GUARD_ODDS_8TH  4
#define GUARD_ODDS_4TH  8

/* The line that goes up over the guard, where it stands, and how long it
   stays there at each of the two speeds. */
#define GUARD_MSG_FLAGS 1
#define GUARD_MSG_STYLE 1
#define GUARD_MSG_X     0x10
#define GUARD_MSG_Y     0xC
#define GUARD_MSG_SLOW  0x3C
#define GUARD_MSG_FAST  0x1E

/* The Persona's arrival, and the frames the guard is held for. */
#define GUARD_MOTION 3
#define GUARD_FRAMES 0x3C

int BtlPersonaGuard(BtlActor *a)
{
    BtlStats *p;
    BtlActor *t;
    int       kind;
    int       move;
    int       odds;
    int       bond;
    int       answers;
    int       i;
    int       j;
    int       speed;
    u_char    mask;

    move = a->move;
    odds = 0;
    if (g_btl_act_kind != 0) {
        return 0;
    }
    if (g_btl_encounter == GUARD_ENCOUNTER_A
        || g_btl_encounter == GUARD_ENCOUNTER_B) {
        return 0;
    }
    i    = 0;
    mask = 1;
    do {
        if ((a->targets & mask) != 0 && i < BTL_PARTY
            && g_btl_actors[i].c.key != 0
            && (signed char)g_btl_actors[i].c.status != BTL_STATUS_DOWN
            && (g_btl_actors[i].flags & BTL_ACTOR_OUT) == 0) {
            answers = 0;
            bond = (g_persona_defs[g_btl_personas[BtlActorPersona(i)].key].bond
                    >> ((g_btl_actors[i].c.key - 1) * PERSONA_BOND_BITS))
                   & PERSONA_BOND_MASK;
            p    = &g_btl_personas[BtlActorPersona(i)];
            kind = p->unk41;
            if (g_btl_actors[i].c.blocked == 0 && bond == PERSONA_BOND_FULL
                && p->slots >= GUARD_SLOTS) {
                switch (kind) {
                case GUARD_SWING:
                    if (move == 0) {
                        answers = 1;
                    }
                    break;
                case GUARD_SPELL:
                    if (move != 0 && move < SPELL_FREE_FIRST) {
                        switch (g_spell_data[move].kind & SPELL_KIND_MASK) {
                        case GUARD_KIND_14:
                        case GUARD_KIND_1A:
                        case GUARD_KIND_1C:
                        case GUARD_KIND_1E:
                        case GUARD_KIND_32:
                            answers = 0;
                            break;
                        default:
                            answers = 1;
                            break;
                        }
                    }
                    break;
                case GUARD_DEMON:
                    if ((u_int)(move - GUARD_DEMON_FIRST) < GUARD_DEMON_COUNT) {
                        answers = 1;
                    }
                    break;
                case GUARD_FREE:
                    if ((u_int)(move - SPELL_FREE_FIRST) < GUARD_FREE_COUNT
                        || (u_int)(move - GUARD_HIGH_FIRST)
                               < GUARD_HIGH_COUNT) {
                        answers = 1;
                    }
                    break;
                case GUARD_ANY:
                    if (move < GUARD_HIGH_FIRST) {
                        switch (g_spell_data[move].kind & SPELL_KIND_MASK) {
                        case GUARD_KIND_14:
                        case GUARD_KIND_1A:
                        case GUARD_KIND_1C:
                        case GUARD_KIND_1E:
                        case GUARD_KIND_32:
                            answers = 0;
                            break;
                        default:
                            answers = 1;
                            break;
                        }
                    } else if (move < GUARD_HIGH_END) {
                        answers = 1;
                    }
                    break;
                }
                if (answers != 0) {
                    if (g_btl_actors[i].c.hp_max / 16
                        >= g_btl_actors[i].c.hp) {
                        odds = GUARD_ODDS_16TH;
                    } else if (g_btl_actors[i].c.hp_max / 8
                               >= g_btl_actors[i].c.hp) {
                        odds = GUARD_ODDS_8TH;
                    } else if (g_btl_actors[i].c.hp_max / 4
                               >= g_btl_actors[i].c.hp) {
                        odds = GUARD_ODDS_4TH;
                    }
                    if (g_btl_debug_guard != 0) {
                        odds = 1;
                    }
                    if ((kind & GUARD_MASK) == GUARD_TYPE && odds != 0
                        && (rand() & (odds - 1)) == 0
                        && g_btl_place_party == 0) {
                        if (g_btl_msg_speed != 2) {
                            BtlOpenMessage(GUARD_MSG_FLAGS, GUARD_MSG_STYLE,
                                           g_btl_msg_persona_acts, GUARD_MSG_X,
                                           GUARD_MSG_Y);
                            if (g_btl_msg_speed == 0) {
                                speed = GUARD_MSG_SLOW;
                            } else {
                                speed = GUARD_MSG_FAST;
                            }
                            g_btl_msg_timer = speed;
                        }
                        t = &g_btl_actors[i];
                        g_btl_effect_obj   = BtlSummonActorPersona(t);
                        g_btl_effect_actor = t;
                        BtlObjSetMotion(g_btl_effect_obj, GUARD_MOTION);
                        j = 0;
                        do {
                            j++;
                            BtlDrawFrame();
                        } while (j < GUARD_FRAMES);
                        BtlCloseMessage(0);
                        return 1;
                    }
                }
            }
        }
        i++;
        mask <<= 1;
    } while (i < BTL_PARTY);
    return 0;
}
