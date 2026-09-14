/* Persona 1 (JP) - the finish the ailment moves share.  BTLP only.
 *   0x800C23CC BtlFxFinish37
 *
 * The finish column's entry for 0x37..0x41, 0x43..0x4B, 0x75..0x83, 0xEE and
 * 0xF2. It walks the acting fighter's targets the way BtlFxFinish01 does and
 * tries each for an ailment. A fighter the effect object is already standing
 * on is stepped over.
 *
 * - 0x44, 0x45 and 0x46 lift BARSAK, MAD and WOLF from a fighter that has it
 *   and put it on one that does not; 0x41 and 0x43 put on CLOAK and COUNTR.
 * - Everything else puts on the ailment its spell record names. A fighter
 *   behind either of the last two wards or BTL_ACTOR_5D is not reached at all,
 *   and neither is one the move's element does nothing to: that fighter's
 *   palette is washed white instead, and unless the move is 0x3F or 0x40 the
 *   acting fighter's object is taken hold of and the ailment kept on it.
 * - Otherwise 0x3F and 0x40 set the two timed conditions for three rounds,
 *   0xEE and 0xF2 roll for GUILT and PANIC at two levels, and the rest roll
 *   against the affinity's answer - weighted by the two fighters' fifth stats
 *   for the moves of element FX_37_STAT_ELEMENT - and put the fighter down with
 *   a cry if the ailment was DEAD.
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
#include <persona/btlp/status.h>

/* The finish's phases after FX_STEP_DONE. */
#define FX_37_APPLY 0x81
#define FX_37_NEXT  0x82
#define FX_37_FIND  0x83
#define FX_37_WAIT  0x84

#define FX_37_WAIT_FRAMES 30

/* The effect sound slots, which the finish gives back one at a time. */
#define FX_37_SLOT_FIRST 7
#define FX_37_SLOT_LAST  12

/* The slot a fallen fighter's cry comes out of: a member's is past the seven
   effect slots, an enemy's is picked by its object. */
#define FX_37_MEMBER_VOICE 7
#define FX_37_ENEMY_VOICE  2

/* The ailments the four toggling moves lift or put on. */
#define FX_37_COUNTR 0x14
#define FX_37_BARSAK 0x15
#define FX_37_MAD    0x16
#define FX_37_WOLF   0x17

/* What keeps a fighter out of reach of an ailment altogether. */
#define FX_37_BLOCKED (BTL_ACTOR_WARD_8E | BTL_ACTOR_WARD_8F | BTL_ACTOR_5D)

/* The element whose chance is weighed by the two fighters' fifth stats, and
   the stat value that makes it certain. */
#define FX_37_STAT_ELEMENT 0x1D
#define FX_37_STAT_SURE    1

/* A roll is a byte: under the chance it lands, and a chance is held to at
   most FX_37_SURE. */
#define FX_37_ROLL 0xFF
#define FX_37_SURE 0x100

/* What 0xEE and 0xF2 roll under, and how deep and how long their ailment is
   put on. */
#define FX_37_MARK_ROLL  0x4D
#define FX_37_MARK_LEVEL 2

/* The motions a struck fighter's object is put on. */
#define FX_37_HURT_MOTION 7
#define FX_37_DOWN_MOTION 8

/* The sounds, and how long the two timed conditions last. */
#define FX_37_SE_SLOT  2
#define FX_37_SE_HIT   5
#define FX_37_SE_NULL  0xA
#define FX_37_SE_DOWN  1
#define FX_37_TIMED_TURNS 3

void BtlFxFinish37(BtlObj *o)
{
    BtlActor *self;
    BtlActor *a;
    int       amount;
    int       n;
    int       voice;
    int       i;
    int       m;

    self = o->actor;
    a = &g_btl_actors[g_btl_hit_slot];
    switch (o->phase) {
    case FX_STEP_DONE:
        BtlFxReopenVoices();
        o->phase++;
        return;
    case FX_37_APPLY:
        voice = g_btl_hit_slot < BTL_PARTY
                    ? g_btl_hit_slot + FX_37_MEMBER_VOICE
                    : (a->obj->unkCD >> 1) + FX_37_ENEMY_VOICE;
        if (g_btl_effect_obj == NULL || g_btl_effect_actor != a) {
            switch (o->kind) {
            case 0x44:
                if ((signed char)a->c.status == FX_37_BARSAK) {
                    a->c.status = 0;
                    a->action = 0;
                } else if (BtlInflictStatus(a, FX_37_BARSAK)) {
                    BtlSePlay(FX_37_SE_SLOT, FX_37_SE_HIT);
                }
                self->unkD0++;
                break;
            case 0x45:
                if ((signed char)a->c.status == FX_37_MAD) {
                    a->c.status = 0;
                    a->action = 0;
                } else if (BtlInflictStatus(a, FX_37_MAD)) {
                    BtlSePlay(FX_37_SE_SLOT, FX_37_SE_HIT);
                }
                self->unkD0++;
                break;
            case 0x46:
                if ((signed char)a->c.status == FX_37_WOLF) {
                    a->c.status = 0;
                } else {
                    if (a->marker == BTL_MARKER_UP || !BtlStatusStops(a)) {
                        a->action = 0;
                    }
                    if (BtlInflictStatus(a, FX_37_WOLF)) {
                        BtlSePlay(FX_37_SE_SLOT, FX_37_SE_HIT);
                    }
                }
                self->unkD0++;
                break;
            case 0x43:
                if (a->marker == BTL_MARKER_UP || !BtlStatusStops(a)) {
                    a->action = 0;
                }
                if (BtlInflictStatus(a, FX_37_COUNTR)) {
                    BtlSePlay(FX_37_SE_SLOT, FX_37_SE_HIT);
                }
                self->unkD0++;
                break;
            case 0x41:
                if (a->marker == BTL_MARKER_UP || !BtlStatusStops(a)) {
                    a->action = 0;
                }
                if (BtlInflictStatus(a, BTL_STATUS_LIFTED)) {
                    BtlSePlay(FX_37_SE_SLOT, FX_37_SE_HIT);
                }
                self->unkD0++;
                break;
            default:
                if (g_spell_data[o->kind].ailment == 0
                    && o->kind != 0x3F && o->kind != 0x40) {
                    break;
                }
                amount = 0;
                if ((a->flags & FX_37_BLOCKED) != 0) {
                    n = BTL_REACT_NULL;
                } else {
                    n = BtlApplyAffinity(&amount,
                                         g_spell_data[o->kind].element,
                                         a->c.unk5C);
                }
                if (g_spell_data[o->kind].element == FX_37_STAT_ELEMENT
                    && a->stat[4] == FX_37_STAT_SURE) {
                    n = 1;
                }
                if (n == BTL_REACT_NULL) {
                    BtlSePlay(FX_37_SE_SLOT, FX_37_SE_NULL);
                    g_btl_clut_fading |= 1 << g_btl_hit_slot;
                    i = 1;
                    do {
                        g_btl_actor_clut[g_btl_hit_slot * FX_CLUT_COLORS + i] =
                            FX_CLUT_WHITE;
                        i++;
                    } while (i < FX_CLUT_COLORS);
                    if ((u_int)(g_btl_fx_move - 0x3F) >= 2) {
                        self->obj->attr |= BTL_OBJ_CARRIED;
                        self->unkCC = g_spell_data[o->kind].ailment;
                    }
                    break;
                }
                if (n <= 0) {
                    break;
                }
                switch (g_btl_fx_move) {
                case 0x3F:
                    a->hit_amount = 0;
                    a->obj->motion = FX_37_HURT_MOTION;
                    a->obj->phase = 0;
                    a->timed_a = FX_37_TIMED_TURNS;
                    a->flags |= BTL_ACTOR_TIMED_A;
                    break;
                case 0x40:
                    a->hit_amount = 0;
                    a->obj->motion = FX_37_HURT_MOTION;
                    a->obj->phase = 0;
                    a->timed_b = FX_37_TIMED_TURNS;
                    a->flags |= BTL_ACTOR_TIMED_B;
                    break;
                case 0xEE:
                    if ((rand() & FX_37_ROLL) < FX_37_MARK_ROLL) {
                      m = a->c.status;
                      if ((u_int)(m - STATUS_PALYZE) >= 2
                          && (signed char)m != STATUS_SICK) {
                        a->c.status = STATUS_GUILT;
                        a->c.ail_level = FX_37_MARK_LEVEL;
                        a->ail_turns = FX_37_MARK_LEVEL;
                        a->hit_amount = 0;
                        a->obj->motion = FX_37_HURT_MOTION;
                        a->obj->phase = 0;
                        BtlObjSetScript(a->obj->mark,
                                        *(const u_long **)(g_btl_actor_gfx
                                                           + STATUS_GUILT * 4
                                                           + BTL_GFX_SCRIPTS));
                        BtlObjSetScript(
                            a->obj->mark->attached,
                            g_btl_ail_level_marks[(signed char)a->c.ail_level]);
                        a->obj->mark->unkCE = STATUS_GUILT + BTL_MARK_BIAS;
                        a->obj->mark->motion = BTL_MARK_MOTION;
                        a->obj->mark->timer = BTL_MARK_TIMER;
                      }
                    }
                    break;
                case 0xF2:
                    if ((rand() & FX_37_ROLL) < FX_37_MARK_ROLL
                        && (u_int)(a->c.status - STATUS_STONE) >= 2) {
                        a->c.status = STATUS_PANIC;
                        a->c.ail_level = FX_37_MARK_LEVEL;
                        a->ail_turns = FX_37_MARK_LEVEL;
                        a->hit_amount = 0;
                        a->obj->motion = FX_37_HURT_MOTION;
                        a->obj->phase = 0;
                        BtlObjSetScript(a->obj->mark,
                                        *(const u_long **)(g_btl_actor_gfx
                                                           + STATUS_PANIC * 4
                                                           + BTL_GFX_SCRIPTS));
                        BtlObjSetScript(
                            a->obj->mark->attached,
                            g_btl_ail_level_marks[(signed char)a->c.ail_level]);
                        a->obj->mark->unkCE = STATUS_PANIC + BTL_MARK_BIAS;
                        a->obj->mark->motion = BTL_MARK_MOTION;
                        a->obj->mark->timer = BTL_MARK_TIMER;
                    }
                    break;
                default:
                    if (g_spell_data[o->kind].element == FX_37_STAT_ELEMENT) {
                        if (a->stat[4] == FX_37_STAT_SURE) {
                            n = FX_37_SURE;
                        } else {
                            n = (self->stat[4] - a->stat[4]) * 3 + n / 4;
                        }
                        if (n >= 0) {
                            m = n;
                            if (m > FX_37_SURE) {
                                m = FX_37_SURE;
                            }
                        } else {
                            m = 0;
                        }
                        n = m;
                    }
                    if ((rand() & FX_37_ROLL) < n
                        && (g_btl_hit_slot >= BTL_PARTY
                            || g_btl_debug_party_immune == 0)
                        && BtlInflictStatus(a, g_spell_data[o->kind].ailment)) {
                        self->unkD0++;
                        if ((signed char)a->c.status == BTL_STATUS_DOWN) {
                            a->hit_amount = 0;
                            a->c.hp = 0;
                            a->obj->motion = FX_37_DOWN_MOTION;
                            a->obj->phase = 0;
                            BtlSePlay(voice, FX_37_SE_DOWN);
                        } else {
                            a->hit_amount = 0;
                            a->obj->motion = FX_37_HURT_MOTION;
                            a->obj->phase = 0;
                            BtlSePlay(FX_37_SE_SLOT, FX_37_SE_HIT);
                        }
                    }
                    break;
                }
                break;
            }
        }
        o->phase++;
        return;
    case FX_37_NEXT:
        g_btl_hits_left--;
        if (g_btl_hit_walk < 0) {
            g_btl_hit_mask = 1;
        } else {
            g_btl_hit_mask <<= 1;
        }
        g_btl_hit_walk++;
        o->phase++;
        return;
    case FX_37_FIND:
        for (; g_btl_hit_walk < BTL_ACTORS;
             g_btl_hit_walk++, g_btl_hit_mask <<= 1) {
            if ((g_btl_actors[g_btl_actor_turn].targets
                 & (u_short)g_btl_hit_mask) != 0
                && g_btl_actors[g_btl_hit_walk].c.key != 0
                && (signed char)g_btl_actors[g_btl_hit_walk].c.status
                       != BTL_STATUS_DOWN
                && (g_btl_actors[g_btl_hit_walk].flags & BTL_ACTOR_OUT) == 0) {
                g_btl_hit_slot = g_btl_hit_walk;
                break;
            }
        }
        if (g_btl_hit_walk >= BTL_ACTORS) {
            o->timer = FX_37_WAIT_FRAMES;
            o->phase++;
        } else {
            o->phase = FX_37_APPLY;
        }
        return;
    case FX_37_WAIT:
        if (o->timer != 0) {
            return;
        }
        i = FX_37_SLOT_FIRST;
        do {
            BtlSoundClose(i);
            i++;
        } while (i < FX_37_SLOT_LAST);
        o->motion = 0;
        o->phase = 0;
        return;
    }
}
