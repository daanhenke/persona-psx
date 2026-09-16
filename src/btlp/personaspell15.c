/* Persona 1 (JP) - spell 0x15 played out by a summoned Persona.  BTLP only.
 *   0x800B1D08 BtlPersonaSpell15
 *
 * One arm of the switch in the routine the persona attack motion hands to,
 * and the only one big enough to stand on its own. It works out what the
 * summon's move does to the fighter it is aimed at - the acting record comes
 * from g_btl_actor_turn and the target from what the aim step left behind -
 * and reaches the spell's own row of g_spell_data for the numbers.
 *
 * The arithmetic is the same shape a member's blow has: the caster's attack
 * and accuracy bent by its own stage moves, the roll for whether the swing
 * lands, the damage weighed against the target's guard, the charm and close
 * ailments taking their share, the build move's own count, a roll of four
 * added on top, and the element weighed against the target's affinities.
 * The target's guard is bent the same way and then never read - the two
 * multiplies that survive it are all that is left of it.
 *
 * What the affinity answers decides the rest: repelled heals the target,
 * nulled turns the blow back on the caster with its palette flashed white,
 * and anything else takes the hp off, plays the hurt and rolls for the ailment
 * the spell carries and for the one a critical leaves. A miss, or a target
 * already spoken for, goes straight to the miss mark.
 *
 * It is arithmetic on doubles, which is what makes it the longest of the arms:
 * the soft-float calls are most of its length.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <rand.h>
#include <libsnd.h>
#include <persona/common/spell.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/cast.h>
#include <persona/btlp/clut.h>
#include <persona/btlp/damage.h>
#include <persona/btlp/debug.h>
#include <persona/btlp/fieldmarks.h>
#include <persona/btlp/object.h>
#include <persona/btlp/round.h>
#include <persona/btlp/sides.h>
#include <persona/btlp/sound.h>
#include <persona/btlp/spellfx.h>
#include <persona/btlp/stats.h>
#include <persona/btlp/status.h>

/* The three moves this arm treats specially: the one that takes its element
   at random, the one that spends the caster outright, and the one that grows
   stronger every time it lands. */
#define SPELL_ANY_ELEMENT 0xDE
#define SPELL_SELF        0xDF
#define SPELL_BUILD       0xDC

/* What the random element is picked between, and where the elements start. */
#define ELEMENT_KINDS 24
#define ELEMENT_FIRST 2

/* A stage move's share, out of eight, and what one level of it comes to. */
#define STAGE_OUT_OF 8
#define STAGE_STEP   2
#define STAGE_BASE   4

/* The build move's count, which caps here and never reaches nought again. */
#define BUILD_MAX 8

/* What a critical is worth, and the spread a landed blow gets on top. */
#define CRIT_TIMES 3
#define SPREAD     4

/* Where a blow is held between, and the two flags that settle it outright:
   a target already spoken for, and the caster's own script. */
#define DAMAGE_CAP     0x270F
#define DAMAGE_DEBUG   0x270F
#define TARGET_SETTLED 0x1900
#define TARGET_HALVED  0x2000

/* The slots the sounds come out of and the seven the arm plays. */
#define SE_SLOT     2
#define VOICE_SLOT  6
#define SE_REPEL    0xB
#define SE_NULL     0xA
#define SE_CRIT     9
#define SE_MISS     8
#define VOICE_HURT  0
#define VOICE_DOWN  1

/* The motions the three answers leave behind. */
#define MOTION_HEAL  0xF
#define MOTION_DOWN  8
#define MOTION_HURT  7
#define MOTION_NULL  0x10
#define MOTION_MISS  0x11

/* Frames the record holds while each of them plays. */
#define HOLD_HURT 0x28
#define HOLD_MISS 0x1E

/* The attribute the caster's record takes while a nulled blow comes back on
   it, and what the miss mark is lifted by. */
#define OBJ_NULLED 0x20000
#define MISS_LIFT  0x180000

/* One time in eight for the spell's own ailment, one in three for the one a
   critical leaves once the blow took a quarter of the target. */
#define AIL_ODDS      8
#define CRIT_AIL      0xB
#define CRIT_AIL_ODDS 3
#define CRIT_AIL_PART 4

/* The mark a record with no slot of its own carries, and the kind the field
   marks are. */
#define MARK_NO_SLOT 0xFF
#define MARK_KIND    0xD

/* Not an exact match yet: everything but two registers is in place - the
   caster's own numbers are held in $a0 in the image and in $v1 here, and
   the mark a miss leaves behind lands in $s3 rather than $s0. The shape,
   the order and the frame are the image's. */
#ifdef NON_MATCHING
void BtlPersonaSpell15(BtlObj *o)
{
    const SpellData *spell;
    const BtlStats  *p;
    BtlActor        *a;
    BtlActor        *t;
    BtlObj          *obj;
    BtlObj          *e;
    /* Sixteen bytes of frame nothing here uses; the routine is the wrong
       length without them. */
    u_long           scratch[4];
    int              damage;
    int              element;
    int              n;
    int              atk;
    int              hit;
    int              def;
    int              react;
    int              crit;
    int              build;
    int              i;

    obj   = g_btl_actors[g_btl_actor_turn].obj;
    a     = obj->actor;
    p     = &g_btl_personas[BtlActorPersona(a->obj->mark_num)];
    spell = &g_spell_data[a->move];
    t     = &g_btl_actors[g_btl_hit_slot];
    if ((short)SsVabTransCompleted(0) == 0) {
        return;
    }

    n   = p->attack;
    atk = n;
    if ((signed char)a->stage[3] != 0) {
        atk = n + n * ((signed char)a->stage[3] * STAGE_STEP + STAGE_BASE)
                      / STAGE_OUT_OF;
    }
    if ((signed char)a->stage[0] != 0) {
        atk -= n * (signed char)a->stage[0] / STAGE_OUT_OF;
    }
    n   = p->accuracy;
    hit = n;
    if ((signed char)a->stage[5] != 0) {
        hit = n + n * ((signed char)a->stage[5] * STAGE_STEP + STAGE_BASE)
                      / STAGE_OUT_OF;
    }
    if ((signed char)a->stage[2] != 0) {
        hit -= n * (signed char)a->stage[2] / STAGE_OUT_OF;
    }
    /* The guard is bent the same way and then never read: only the two
       multiplies are left of it, which gcc keeps because they write HI and
       LO. */
    n = t->defence;
    if ((signed char)t->stage[4] != 0) {
        def = n + n * ((signed char)t->stage[4] * STAGE_STEP + STAGE_BASE)
                      / STAGE_OUT_OF;
    }
    if ((signed char)t->stage[1] != 0) {
        def -= n * (signed char)t->stage[1] / STAGE_OUT_OF;
    }

    if (a->move == SPELL_ANY_ELEMENT) {
        element = rand() % ELEMENT_KINDS + ELEMENT_FIRST;
    } else {
        element = spell->element;
    }

    if (t->unkD3 == 0
        && (g_btl_debug_flags[2] != 0
            || (a->flags & BTL_ACTOR_SCRIPT_DONE) != 0
            || BtlRollHit(g_btl_party_counted, hit, (signed char)a->c.status,
                          g_btl_enemy_counted, t->evade,
                          (signed char)t->c.status)
                   != 0)) {
        if (a->move == SPELL_SELF) {
            damage = a->c.hp * 2;
        } else {
            damage = BtlDamageBoosted(atk, (u_char)(signed char)spell->power,
                                      t->defence);
        }
        if ((signed char)a->c.status == BTL_STATUS_CHARM) {
            damage /= BTL_STATUS_CHARM - (signed char)a->c.ail_level;
        }
        if (BtlRollCritical(a, t) != 0) {
            crit = 1;
            damage *= CRIT_TIMES;
        }
        if ((signed char)a->c.status == BTL_STATUS_CLOSE
            && (signed char)a->c.ail_level > 0) {
            damage /= 2;
        }
        if (a->move == SPELL_BUILD) {
            /* The count through a byte of its own, the clamps included. */
            build = a->build + 1;
            a->build = build;
            if (a->build != 0) {
                if ((u_char)build > BUILD_MAX) {
                    build = BUILD_MAX;
                    a->build = build;
                }
            } else {
                build = 1;
                a->build = build;
            }
            damage += damage * a->build / STAGE_OUT_OF;
        }
        damage += rand() & (SPREAD - 1);
        if ((t->flags & TARGET_SETTLED) != 0) {
            react = BTL_REACT_NULL;
        } else {
            react = BtlApplyAffinity(&damage, element, t->c.resist);
        }
        if (g_btl_debug_flags[2] != 0) {
            react = 0;
            damage = DAMAGE_DEBUG;
        }
        if ((a->flags & BTL_ACTOR_SCRIPT_DONE) != 0) {
            react = 0;
            damage = t->c.hp;
        }
        damage = damage < 0 ? 0 : damage > DAMAGE_CAP ? DAMAGE_CAP : damage;

        switch (react) {
        case BTL_REACT_REPEL:
            BtlSePlay(SE_SLOT, SE_REPEL);
            o->child = BtlSpawnMoveStrike(a->move, 0, &t->obj->x);
            /* Added on the record and then read back off it for the clamp:
               the store to the hit amount makes gcc load it again. */
            t->c.hp += damage;
            n = t->c.hp;
            t->hit_amount = damage;
            if (t->c.hp_max < n) {
                n = t->c.hp_max;
            }
            t->c.hp = n;
            t->obj->motion = MOTION_HEAL;
            break;

        case BTL_REACT_NULL:
            obj->attr |= OBJ_NULLED;
            BtlSoundClose(VOICE_SLOT);
            BtlSoundOpen(g_btl_banks, VOICE_SLOT, obj->kind);
            BtlSePlay(SE_SLOT, SE_NULL);
            g_btl_clut_fading |= 1 << g_btl_hit_slot;
            i = 1;
            do {
                g_btl_actor_clut[g_btl_hit_slot * FX_CLUT_COLORS + i] =
                    FX_CLUT_WHITE;
                i++;
            } while (i < FX_CLUT_COLORS);
            o->child = BtlSpawnMoveStrike(a->move, 1, &obj->x);
            a->hit_amount = damage;
            a->c.hp -= damage;
            if (g_btl_debug_flags[1] != 0 && a->c.hp <= 0) {
                a->c.hp = 1;
            }
            if (a->c.hp <= 0) {
                a->resume_motion = obj->motion;
                a->resume_phase = obj->phase;
                a->c.hp = 0;
                obj->motion = MOTION_DOWN;
                BtlSePlay(VOICE_SLOT, VOICE_DOWN);
                g_btl_hits_left = 0;
            } else {
                a->unkDF = 1;
                a->resume_motion = obj->motion;
                a->resume_phase = obj->phase;
                obj->motion = MOTION_NULL;
                BtlSePlay(VOICE_SLOT, VOICE_HURT);
            }
            o->timer = HOLD_HURT;
            obj->phase = 0;
            break;

        default:
            if ((t->flags & TARGET_HALVED) != 0) {
                damage /= 2;
            }
            o->child = BtlSpawnMoveStrike(a->move, 0, &t->obj->x);
            if (damage == 0) {
                break;
            }
            t->hit_amount = damage;
            t->c.hp -= damage;
            a->damage_dealt += damage;
            if (crit != 0) {
                BtlSePlay(SE_SLOT, SE_CRIT);
            }
            if (t->c.hp <= 0) {
                a->damage_dealt += t->c.hp;
                t->c.hp = 0;
                t->obj->motion = MOTION_DOWN;
                BtlSePlay(VOICE_SLOT, VOICE_DOWN);
            } else {
                t->obj->motion = MOTION_HURT;
                BtlSePlay(VOICE_SLOT, VOICE_HURT);
                if (spell->ailment != 0 && (rand() & (AIL_ODDS - 1)) == 0) {
                    BtlInflictStatus(t, spell->ailment);
                }
                if (crit != 0 && damage >= t->c.hp_max / CRIT_AIL_PART
                    && rand() % CRIT_AIL_ODDS == 0) {
                    BtlInflictStatus(t, CRIT_AIL);
                }
            }
            t->obj->phase = 0;
            break;
        }
    } else {
        BtlSePlay(SE_SLOT, SE_MISS);
        o->child = BtlSpawnMoveStrike(a->move, 0, &t->obj->x);
        t->obj->motion = MOTION_MISS;
        o->timer = HOLD_MISS;
        /* The mark back through the local the caster's own record came in:
           nothing here reads that again, and the two of them in registers of
           their own is one held register too many. */
        obj = BtlSpawnMiss(&t->obj->x);
        obj->mark_num = MARK_NO_SLOT;
        obj->z -= MISS_LIFT;
        e = g_btl_obj_pool;
        while (e != 0) {
            if (e->kind == MARK_KIND && e->mark_num == t->obj->mark_num) {
                BtlObjMoveBefore(obj, e);
                goto placed;
            }
            e = e->next;
        }
        obj->mark_num = t->obj->mark_num;
    placed:;
    }
    o->phase++;
}
#else
INCLUDE_ASM("btlp/nonmatchings/personaspell15", BtlPersonaSpell15);
#endif
