/* Persona 1 (JP) - a fighter shaken by a blow.  BTLP only.
 *   0x8008B394 BtlActorMotion07
 *
 * Motion 7 of both fighter tables. Nothing happens until the record is free to
 * move and the sound bank has finished loading.
 *
 * The first frame lands the blow. A sleeping fighter that took damage may be
 * shaken awake: half the time it loses a level of the sleep, and once the
 * level falls below nought the sleep and the action it was waiting on are
 * over; a level left over keeps it asleep two more turns. The fighter is put
 * on its side's reaction script, the amount goes up over it as a still number
 * that replaces any still number already there, and it is tinted - from the
 * pick tint table by its child count where attribute 0x200000 asks, and
 * otherwise to half grey with its palette tinted the pick colour. Then it is
 * held.
 *
 * While the hold runs the fighter shakes two pixels either side of where it
 * stands, and a member's marker is washed white and shakes one pixel with it.
 * When it is over the palettes are put back from their base, the fighter goes
 * back to the motion it was taken out of if it was being carried and to idle
 * if not, and it is stood back up: an enemy in its flinch or its spawn pose, a
 * member in its weak, flinched or ordinary stance.
 */
#include <decomp/types.h>
#include <decomp/libc.h>
#include <libsnd.h>
#include <rand.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/clut.h>
#include <persona/btlp/fieldmarks.h>
#include <persona/btlp/model.h>
#include <persona/btlp/object.h>
#include <persona/btlp/round.h>

/* The two attribute bits that mean the record is not free to move yet. */
#define HURT_BUSY (BTL_OBJ_HELD | BTL_OBJ_TRACKING)

/* The ailment a blow can shake a fighter out of, and how many more turns of it
   are left when it does not. */
#define HURT_SLEEP      7
#define HURT_SLEEP_LEFT 2

/* How high a member's number is put, and what a number stands for while the
   old one is searched for. */
#define HURT_NUMBER_Z     (-0x300000)
#define HURT_NUMBER_ALONE 0xFF

/* The tint: the attribute that asks for the table, the grey otherwise, and how
   fast either is taken on. */
#define HURT_TINT_TABLE 0x200000
#define HURT_TINT_GREY  0x80
#define HURT_TINT_FADE  0xFF

#define HURT_HOLD 30

/* How far the fighter and its marker are shaken each way, in 16.16. */
#define HURT_SHAKE      0x20000
#define HURT_MARK_SHAKE 0x10000
#define MARKER_WHITE    0x2000000

/* The motion a carried fighter is on, and the scripts a member stands back up
   in: weak, flinched, and ordinary. */
#define HURT_CARRIED_MOTION 0x10
#define SCRIPT_STAND 0
#define SCRIPT_WEAK  6
#define SCRIPT_HIT   8

void BtlActorMotion07(BtlObj *o)
{
    BtlObj       *num;
    BtlObj       *p;
    const u_char *row;
    int           script;

    if ((o->attr & HURT_BUSY) != 0) {
        return;
    }
    if ((short)SsVabTransCompleted(0) == 0) {
        return;
    }
    switch (o->phase) {
    case 0:
        if ((signed char)o->actor->c.status == HURT_SLEEP
            && o->actor->hit_amount != 0 && (rand() & 1) != 0) {
            if (--*(signed char *)&o->actor->c.ail_level < 0) {
                o->actor->c.status = 0;
                o->actor->c.ail_level = 0;
                o->actor->action = 0;
            } else {
                o->actor->ail_turns = HURT_SLEEP_LEFT;
            }
        }
        if ((o->attr & BTL_OBJ_OTHER_SIDE) != 0) {
            BtlObjSetScript(o,
                (BtlSeqStep *)o->scripts[g_btl_models[o->kind].hurt]);
            if (g_btl_actors[o->mark_num].hit_amount != 0) {
                num = BtlSpawnHitNumber(g_btl_actors[o->mark_num].hit_amount,
                                        &o->x, HIT_NUMBER_STILL);
                num->z = (g_btl_models[o->kind].number_z << 16) + o->z;
            }
        } else {
            row = &g_btl_talk_motion[o->kind * MEMBER_SCRIPT_MODEL];
            BtlObjSetScript(o, (BtlSeqStep *)o->scripts[
                row[o->actor->script_pick * MEMBER_SCRIPT_PICK]]);
            if (g_btl_actors[o->mark_num].hit_amount != 0) {
                num = BtlSpawnHitNumber(g_btl_actors[o->mark_num].hit_amount,
                                        &o->x, HIT_NUMBER_STILL);
                num->z = HURT_NUMBER_Z;
                BtlBuildMarkers();
            }
        }
        if (g_btl_actors[o->mark_num].hit_amount != 0) {
            num->mark_num = HURT_NUMBER_ALONE;
            for (p = g_btl_obj_pool; p != NULL; p = p->next) {
                if (p->kind == MARK_KIND_STILL && p->mark_num == o->mark_num) {
                    BtlObjFree(p);
                    break;
                }
            }
            num->mark_num = o->mark_num;
        }
        if ((o->attr & HURT_TINT_TABLE) == 0 || o->children == 0) {
            o->rgb_to[0] = HURT_TINT_GREY;
            o->rgb_to[1] = HURT_TINT_GREY;
            o->rgb_to[2] = HURT_TINT_GREY;
            o->rgb[0] = HURT_TINT_GREY;
            o->rgb[1] = HURT_TINT_GREY;
            o->rgb[2] = HURT_TINT_GREY;
            BtlTintActorClut(o->mark_num, g_btl_tint_pick_r[0],
                             g_btl_tint_pick_g[0], g_btl_tint_pick_b[0]);
        } else {
            o->rgb_to[0] = g_btl_tint_pick_r[o->children * 3];
            o->rgb_to[1] = g_btl_tint_pick_g[o->children * 3];
            o->rgb_to[2] = g_btl_tint_pick_b[o->children * 3];
            o->fade = HURT_TINT_FADE;
        }
        o->timer = HURT_HOLD;
        o->phase++;
        break;

    case 1:
        num = g_btl_marker_obj[o->mark_num];
        if (o->timer == 0) {
            o->rgb_to[0] = HURT_TINT_GREY;
            o->rgb_to[1] = HURT_TINT_GREY;
            o->rgb_to[2] = HURT_TINT_GREY;
            o->fade = HURT_TINT_FADE;
            o->attr &= ~HURT_TINT_TABLE;
            o->x = o->x2;
            memcpy((u_char *)g_btl_actor_clut + o->mark_num * BTL_CLUT_BYTES,
                   (u_char *)g_btl_actor_clut_base
                       + o->mark_num * BTL_CLUT_BYTES,
                   BTL_CLUT_BYTES);
            memcpy((u_char *)g_btl_actor_clut_to + o->mark_num * BTL_CLUT_BYTES,
                   (u_char *)g_btl_actor_clut_base
                       + o->mark_num * BTL_CLUT_BYTES,
                   BTL_CLUT_BYTES);
            if (o->motion == HURT_CARRIED_MOTION) {
                o->motion = o->actor->resume_motion;
                o->phase = o->actor->resume_phase;
                o->attr &= ~BTL_OBJ_CARRIED;
                o->actor->unkCC = 0;
            } else {
                o->motion = 0;
                o->phase = 0;
            }
            if ((o->attr & BTL_OBJ_OTHER_SIDE) != 0) {
                if ((o->actor->flags & BTL_ACTOR_FLINCHED) != 0) {
                    script = g_btl_models[o->kind].hit;
                } else {
                    script = g_btl_models[o->kind].spawn;
                }
            } else {
                num->attr &= ~MARKER_WHITE;
                BtlObjSetPos(num, num->x2, num->y2, num->z2);
                if (o->actor->c.hp_max / 4 >= o->actor->c.hp) {
                    row = &g_btl_member_scripts[SCRIPT_WEAK
                                                + o->kind * MEMBER_SCRIPT_MODEL];
                } else if ((o->actor->flags & BTL_ACTOR_FLINCHED) != 0) {
                    row = &g_btl_member_scripts[SCRIPT_HIT
                                                + o->kind * MEMBER_SCRIPT_MODEL];
                } else {
                    row = &g_btl_member_scripts[SCRIPT_STAND
                                                + o->kind * MEMBER_SCRIPT_MODEL];
                }
                script = row[o->actor->script_pick * MEMBER_SCRIPT_PICK];
            }
            BtlObjSetScript(o, (BtlSeqStep *)o->scripts[script]);
            BtlObjStatusTint(o);
        } else {
            o->x = o->x2 + ((o->timer & 1) ? -HURT_SHAKE : HURT_SHAKE);
            if ((o->attr & BTL_OBJ_OTHER_SIDE) == 0) {
                num->attr |= MARKER_WHITE;
                BtlObjSetPos(num,
                    num->x2 + ((o->timer & 1) ? -HURT_MARK_SHAKE
                                               : HURT_MARK_SHAKE),
                    num->y2, num->z2);
            }
        }
        break;
    }
}
