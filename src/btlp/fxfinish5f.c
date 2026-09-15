/* Persona 1 (JP) - the finish the recovery moves share.  BTLP only.
 *   0x800C17E8 BtlFxFinish5F
 *
 * The finish column's entry for 0x5F..0x6A, 0xED, 0xF1 and 0xF3..0xF6. Like
 * BtlFxFinish01 it walks the acting fighter's targets one at a time, and at
 * each it applies the move instead of resolving a blow. A fighter held by
 * PUPPET is stepped over.
 *
 * - 0x5F and 0x62 heal the healer's unk3A and up to fifteen more, 0x60 and 0x63
 *   twice that, and a healer shut in by CLOSE gives only half. The fighter's
 *   object is put on its healing motion and the heal is put up over it.
 * - 0x61 and 0x64 heal whatever hp the fighter is missing - unless a Persona
 *   is acting, when the Persona's unk41 picks one of six restorations of hp,
 *   sp or both, the last three of which lift the ailment too.
 * - 0x65 lifts the first twelve ailments outright and 0x66 steps them down a
 *   level; 0x67..0x6A lift POISON, PALYZE, STONE and SICK.
 * - 0xED gives back an eighth of the hp and some sp, 0xF6 a sixteenth of the
 *   sp, 0xF1 and 0xF4 everything - lifting every ailment short of POISON or of
 *   DEAD - and 0xF5 sets BTL_ACTOR_F5.
 * - 0xF3 gives a little hp back, or on a failed roll halves the hp and
 *   poisons the fighter.
 *
 * BTL_ACTOR_TIMED_A stops every hp restoration.
 */
#include <decomp/types.h>
#include <rand.h>
#include <persona/common/spell.h>
#include <persona/common/status.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/damage.h>
#include <persona/btlp/object.h>
#include <persona/btlp/round.h>
#include <persona/btlp/sound.h>
#include <persona/btlp/spellfx.h>
#include <persona/btlp/stats.h>
#include <persona/btlp/status.h>

/* The finish's phases after FX_STEP_DONE. */
#define FX_5F_APPLY 0x81
#define FX_5F_NEXT  0x82
#define FX_5F_FIND  0x83
#define FX_5F_DONE  0x84

#define FX_5F_WAIT_FRAMES 30

/* Where hp is held, and the most a heal can come to. */
#define FX_5F_HP_MIN 1
#define FX_5F_HP_CAP 999

/* What 0xF3 rolls against, and the ailment it leaves on a failed roll. */
#define FX_5F_F3_LIMIT 150

/* The motion a healed fighter's object is put on, and the sounds. */
#define FX_5F_HEAL_MOTION 0xF
#define FX_5F_SE_SLOT     2
#define FX_5F_SE_HEAL     0xB
#define FX_5F_SE_POISON   5

/* The act kind under which the acting fighter's Persona picks the restoration
   0x61 and 0x64 give. */
#define FX_5F_ACT_BY_PERSONA 2

/* A healer shut in by CLOSE, whose object is not on motion 3, gives half. */
#define FX_5F_HEALER_CLOSED(self)                                          \
    ((self)->obj->motion != 3                                              \
     && (signed char)(self)->c.status == STATUS_CLOSE                      \
     && (signed char)(self)->c.ail_level > 0)

void BtlFxFinish5F(BtlObj *o)
{
    BtlActor *self;
    BtlActor *a;
    int       amount;
    int       chance;
    int       n;

    self = o->actor;
    a = &g_btl_actors[g_btl_hit_slot];
    switch (o->phase) {
    case FX_STEP_DONE:
        o->phase++;
        /* fall through */
    case FX_5F_APPLY:
        amount = 0;
        BtlApplyAffinity(&amount, g_spell_data[g_btl_fx_move].element,
                         a->c.resist);
        if ((signed char)a->c.status != BTL_STATUS_NOINPUT) {
            switch (o->kind) {
            case 0xF3:
                if ((a->flags & BTL_ACTOR_TIMED_A) != 0) {
                    break;
                }
                chance = a->stat[4] >> 1;
                if (chance == 0) {
                    chance = 1;
                }
                if (a->c.hp >= a->c.hp_max * FX_5F_F3_LIMIT / 100
                    || rand() % chance == 0) {
                    a->c.hp = a->c.hp_max / 2;
                    if (BtlInflictStatus(a, STATUS_POISON)) {
                        BtlSePlay(FX_5F_SE_SLOT, FX_5F_SE_POISON);
                    }
                } else {
                    a->c.hp += a->c.level / 2 + rand() % 8;
                    self->unkD0++;
                }
                if (a->c.hp > 0) {
                    n = a->c.hp;
                    if (n > FX_5F_HP_CAP) {
                        n = FX_5F_HP_CAP;
                    }
                } else {
                    n = FX_5F_HP_MIN;
                }
                a->c.hp = n;
                break;
            case 0xF1:
                if ((a->flags & BTL_ACTOR_TIMED_A) == 0) {
                    a->c.hp = a->c.hp_max;
                }
                a->c.sp = a->c.sp_max;
                self->unkD0++;
                if ((signed char)a->c.status < STATUS_POISON) {
                    if (a->marker == BTL_MARKER_UP || !BtlStatusStops(a)) {
                        a->action = 0;
                    }
                    a->c.status = 0;
                    a->c.ail_level = 0;
                }
                break;
            case 0xF4:
                if ((a->flags & BTL_ACTOR_TIMED_A) == 0) {
                    a->c.hp = a->c.hp_max;
                }
                a->c.sp = a->c.sp_max;
                self->unkD0++;
                if ((signed char)a->c.status < BTL_STATUS_DOWN) {
                    if (a->marker == BTL_MARKER_UP || !BtlStatusStops(a)) {
                        a->action = 0;
                    }
                    a->c.status = 0;
                    a->c.ail_level = 0;
                }
                break;
            case 0xED:
                if ((a->flags & BTL_ACTOR_TIMED_A) == 0) {
                    a->c.hp += a->c.hp_max / 8 + rand() % 8;
                    a->c.hp = a->c.hp > a->c.hp_max ? a->c.hp_max : a->c.hp;
                }
                a->c.sp += a->c.level / 4 + rand() % 8 * 2;
                a->c.sp = a->c.sp > a->c.sp_max ? a->c.sp_max : a->c.sp;
                self->unkD0++;
                break;
            case 0xF5:
                if ((a->flags & BTL_ACTOR_TIMED_A) != 0) {
                    break;
                }
                a->flags |= BTL_ACTOR_F5;
                self->unkD0++;
                break;
            case 0xF6:
                a->c.sp += a->c.sp_max / 16;
                a->c.sp = a->c.sp > a->c.sp_max ? a->c.sp_max : a->c.sp;
                self->unkD0++;
                break;
            case 0x5F:
            case 0x62:
                amount = (a->flags & BTL_ACTOR_TIMED_A) == 0
                             ? g_btl_actors[g_btl_actor_turn].unk3A
                                   + (rand() & 0xF)
                             : 0;
                if (FX_5F_HEALER_CLOSED(self)) {
                    amount = (u_int)amount >> 1;
                }
                self->unkD0++;
                a->c.hp += amount;
                a->hit_amount = amount;
                a->c.hp = a->c.hp > a->c.hp_max ? a->c.hp_max : a->c.hp;
                a->resume_motion = a->obj->motion;
                a->resume_phase = a->obj->phase;
                a->obj->motion = FX_5F_HEAL_MOTION;
                a->obj->phase = 0;
                BtlSePlay(FX_5F_SE_SLOT, FX_5F_SE_HEAL);
                break;
            case 0x60:
            case 0x63:
                amount = (a->flags & BTL_ACTOR_TIMED_A) == 0
                             ? (g_btl_actors[g_btl_actor_turn].unk3A
                                + (rand() & 0xF)) * 2
                             : 0;
                if (FX_5F_HEALER_CLOSED(self)) {
                    amount = (u_int)amount >> 1;
                }
                self->unkD0++;
                a->c.hp += amount;
                a->hit_amount = amount;
                a->c.hp = a->c.hp > a->c.hp_max ? a->c.hp_max : a->c.hp;
                a->resume_motion = a->obj->motion;
                a->resume_phase = a->obj->phase;
                a->obj->motion = FX_5F_HEAL_MOTION;
                a->obj->phase = 0;
                BtlSePlay(FX_5F_SE_SLOT, FX_5F_SE_HEAL);
                break;
            case 0x61:
            case 0x64:
                if (g_btl_act_kind == FX_5F_ACT_BY_PERSONA) {
                    /* Eight bytes of locals nothing writes: they are what
                       gives the image its frame, a slot above `amount`. */
                    long unused[2];
                    switch (g_btl_personas[BtlActorPersona(g_btl_actor_turn)]
                                .unk41 & 0xF) {
                    case 0:
                        amount = a->c.sp_max / 2;
                        amount = amount >= 0
                                     ? (amount > a->c.sp_max - a->c.sp
                                            ? a->c.sp_max - a->c.sp : amount)
                                     : 0;
                        a->c.sp += amount;
                        amount = 0;
                        break;
                    case 1:
                        amount = a->c.hp_max / 2;
                        amount = amount >= 0
                                     ? (amount > a->c.hp_max - a->c.hp
                                            ? a->c.hp_max - a->c.hp : amount)
                                     : 0;
                        a->c.hp += amount;
                        break;
                    case 2:
                        amount = a->c.sp_max / 2;
                        amount = amount >= 0
                                     ? (amount > a->c.sp_max - a->c.sp
                                            ? a->c.sp_max - a->c.sp : amount)
                                     : 0;
                        a->c.sp += amount;
                        amount = a->c.hp_max / 2;
                        amount = amount >= 0
                                     ? (amount > a->c.hp_max - a->c.hp
                                            ? a->c.hp_max - a->c.hp : amount)
                                     : 0;
                        a->c.hp += amount;
                        break;
                    case 3:
                        amount = a->c.sp_max - a->c.sp;
                        a->c.sp += amount;
                        a->c.status = 0;
                        a->c.ail_level = 0;
                        amount = 0;
                        break;
                    case 4:
                        amount = a->c.hp_max - a->c.hp;
                        a->c.hp += amount;
                        a->c.status = 0;
                        a->c.ail_level = 0;
                        break;
                    case 5:
                        a->c.sp = a->c.sp_max;
                        amount = a->c.hp_max - a->c.hp;
                        a->c.hp += amount;
                        a->c.status = 0;
                        a->c.ail_level = 0;
                        break;
                    }
                    if (amount != 0) {
                        self->unkD0++;
                        a->hit_amount = amount;
                        a->resume_motion = a->obj->motion;
                        a->resume_phase = a->obj->phase;
                        a->obj->motion = FX_5F_HEAL_MOTION;
                        a->obj->phase = 0;
                        BtlSePlay(FX_5F_SE_SLOT, FX_5F_SE_HEAL);
                    }
                } else {
                    amount = (a->flags & BTL_ACTOR_TIMED_A) == 0
                                 ? a->c.hp_max - a->c.hp
                                 : 0;
                    if (FX_5F_HEALER_CLOSED(self)) {
                        amount /= 2;
                    }
                    self->unkD0++;
                    a->c.hp += amount;
                    a->hit_amount = amount;
                    a->resume_motion = a->obj->motion;
                    a->resume_phase = a->obj->phase;
                    a->obj->motion = FX_5F_HEAL_MOTION;
                    a->obj->phase = 0;
                    BtlSePlay(FX_5F_SE_SLOT, FX_5F_SE_HEAL);
                }
                break;
            case 0x65:
            case 0x66:
                if ((u_int)(a->c.status - 1) < 12) {
                    self->unkD0++;
                    if (o->kind == 0x65) {
                        if (a->marker == BTL_MARKER_UP || !BtlStatusStops(a)) {
                            a->action = 0;
                        }
                        a->c.status = 0;
                        a->c.ail_level = 0;
                        break;
                    }
                    if (a->marker == BTL_MARKER_UP || !BtlStatusStops(a)) {
                        a->action = 0;
                    }
                    /* Stepped down as a signed byte: through the u_char the
                       constant is folded to 255. */
                    if (--*(signed char *)&a->c.ail_level < 0) {
                        a->c.status = 0;
                        a->c.ail_level = 0;
                    }
                }
                break;
            case 0x67:
                if ((signed char)a->c.status == STATUS_POISON) {
                    self->unkD0++;
                    if (a->marker == BTL_MARKER_UP || !BtlStatusStops(a)) {
                        a->action = 0;
                    }
                    a->c.status = 0;
                    a->c.ail_level = 0;
                }
                break;
            case 0x68:
                if ((signed char)a->c.status == STATUS_POISON + 1) {
                    self->unkD0++;
                    if (a->marker == BTL_MARKER_UP || !BtlStatusStops(a)) {
                        a->action = 0;
                    }
                    a->c.status = 0;
                    a->c.ail_level = 0;
                }
                break;
            case 0x69:
                if ((signed char)a->c.status == STATUS_POISON + 2) {
                    self->unkD0++;
                    if (a->marker == BTL_MARKER_UP || !BtlStatusStops(a)) {
                        a->action = 0;
                    }
                    a->c.status = 0;
                    a->c.ail_level = 0;
                }
                break;
            case 0x6A:
                if ((signed char)a->c.status == STATUS_POISON + 3) {
                    self->unkD0++;
                    if (a->marker == BTL_MARKER_UP || !BtlStatusStops(a)) {
                        a->action = 0;
                    }
                    a->c.status = 0;
                    a->c.ail_level = 0;
                }
                break;
            }
        }
        o->phase++;
        return;
    case FX_5F_NEXT:
        if (o->timer != 0) {
            return;
        }
        g_btl_hits_left--;
        if (g_btl_hit_walk < 0) {
            g_btl_hit_mask = 1;
        } else {
            g_btl_hit_mask <<= 1;
        }
        g_btl_hit_walk++;
        o->phase++;
        return;
    case FX_5F_FIND:
        for (; g_btl_hit_walk < BTL_ACTORS;
             g_btl_hit_walk++, g_btl_hit_mask <<= 1) {
            if ((g_btl_actors[g_btl_actor_turn].targets
                 & g_btl_hit_mask) != 0
                && g_btl_actors[g_btl_hit_walk].c.key != 0
                && (signed char)g_btl_actors[g_btl_hit_walk].c.status
                       != BTL_STATUS_DOWN
                && (g_btl_actors[g_btl_hit_walk].flags & BTL_ACTOR_OUT) == 0) {
                g_btl_hit_slot = g_btl_hit_walk;
                break;
            }
        }
        if (g_btl_hit_walk >= BTL_ACTORS) {
            o->timer = FX_5F_WAIT_FRAMES;
            o->phase++;
        } else {
            o->phase = FX_5F_APPLY;
        }
        return;
    case FX_5F_DONE:
        o->motion = 0;
        o->phase = 0;
        return;
    }
}
