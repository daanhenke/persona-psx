/* Persona 1 (JP) - the motions a fighter takes a blow in.  BTLP only.
 *   0x8008CBC0 BtlMemberMotion0F  0x8008CDA0 BtlActorMotion04
 *   0x8008CE88 BtlMemberMotion11
 *
 * Entries of g_btl_member_motion and g_btl_enemy_motion, named for the motion
 * they answer to the way the rest of both tables are. Motion 4 is the one both
 * sides share.
 *
 * BtlActorMotion04 is the flinch on its own: the record is put on its hit
 * script and let go straight back to idle. A member takes the script out of
 * its row of g_btl_member_scripts and is flagged as having taken it, which is
 * what stops motion 0x11 standing it back up again; an enemy's comes from its
 * model.
 *
 * BtlMemberMotion11 is the same flinch held for HIT_HOLD frames, after which
 * the fighter is put back on the script it stands in - unless it has been
 * flagged in the meantime - and let go.
 *
 * BtlMemberMotion0F puts the amount the blow took over the fighter and holds
 * it there. Nothing happens until the record is free to move and the sound
 * bank has finished loading. The number is spawned where the fighter stands -
 * an enemy's at the height its model gives, a member's at a fixed one, which
 * also has the markers rebuilt - and is put into the object list in front of
 * any number already up over the same fighter, so the newest draws on top.
 * Once the hold is over the fighter goes on with the motion it was taken out
 * of if the whole side is acting or it is its own turn, and goes idle if not.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <libsnd.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/model.h>
#include <persona/btlp/number.h>
#include <persona/btlp/object.h>
#include <persona/btlp/round.h>

/* The attribute bit that says the record belongs to the other side, and the
   two that mean it is not free to move yet. */
#define HIT_OTHER_SIDE 0x200
#define HIT_BUSY       (BTL_OBJ_HELD | BTL_OBJ_TRACKING)

/* The number's kind as it is spawned, the kind a number record ends up with,
   and the height a member's is put at. */
#define HIT_NUMBER_SPAWN 0x24
#define HIT_NUMBER_KIND  0x12
#define HIT_NUMBER_Z     (-0x300000)
#define HIT_NUMBER_ALONE 0xFF

/* How long a flinch or a number is held. */
#define HIT_HOLD 30

/* Two of a member's ten scripts: the one it stands in, and its flinch. */
#define MEMBER_SCRIPT_MODEL 0x28
#define MEMBER_SCRIPT_PICK  10
#define SCRIPT_STAND        0
#define SCRIPT_HIT          8


/* Set on a member that has flinched, and the act kind that has the whole side
   acting at once. */
#define ACTOR_FLINCHED 0x2000
#define ACT_KIND_SIDE  2

void BtlMemberMotion0F(BtlObj *o)
{
    BtlObj *num;
    BtlObj *p;

    if ((o->attr & HIT_BUSY) != 0) {
        return;
    }
    if ((short)SsVabTransCompleted(0) == 0) {
        return;
    }
    switch (o->phase) {
    case 0:
        num = BtlSpawnHitNumber(g_btl_actors[o->mark_num].hit_amount, &o->x,
                                HIT_NUMBER_SPAWN);
        if (o->attr & HIT_OTHER_SIDE) {
            num->z = (g_btl_models[o->kind].number_z << 16) + o->z;
        } else {
            num->z = HIT_NUMBER_Z;
            BtlBuildMarkers();
        }
        num->mark_num = HIT_NUMBER_ALONE;
        for (p = g_btl_obj_pool; p != NULL; p = p->next) {
            if (p->kind == HIT_NUMBER_KIND && p->mark_num == o->mark_num) {
                BtlObjMoveBefore(p, num);
                break;
            }
        }
        num->mark_num = o->mark_num;
        o->timer = HIT_HOLD;
        o->phase++;
        break;
    case 1:
        if (o->timer != 0) {
            break;
        }
        if (g_btl_act_kind == ACT_KIND_SIDE
            || g_btl_actor_turn == o->mark_num) {
            o->motion = o->actor->unkC5;
            o->phase = o->actor->unkC6;
        } else {
            o->motion = 0;
            o->phase = 0;
        }
        break;
    }
}

/* 99.05%, registers only: the table's address and the actor's pick come out
   in each other's registers, and the last add with its operands the other way
   round. The index through a local, and its terms turned round, change
   nothing or move further away. */
#ifdef NON_MATCHING
void BtlActorMotion04(BtlObj *o)
{
    int script;

    if (o->attr & HIT_OTHER_SIDE) {
        BtlObjSetScript(o, (BtlSeqStep *)o->scripts[g_btl_models[o->kind].hit]);
        o->motion = 0;
    } else {
        script = g_btl_member_scripts[SCRIPT_HIT
                                      + o->kind * MEMBER_SCRIPT_MODEL
                                      + o->actor->script_pick
                                            * MEMBER_SCRIPT_PICK];
        BtlObjSetScript(o, (BtlSeqStep *)o->scripts[script]);
        o->actor->flags |= ACTOR_FLINCHED;
        o->motion = 0;
    }
    o->phase = 0;
}
#else
INCLUDE_ASM("btlp/nonmatchings/hitmotion", BtlActorMotion04);
#endif

/* 93.85%: the stand script is slot 0 of the member's row, and with no constant
   in the index gcc sums the model's block and the pick first and loads
   symbol-relative, where the image adds the table to the model's block before
   the pick - as it does for the flinch, whose slot is not 0. An explicit
   address, pointer arithmetic and the terms turned round all fold the same
   way. */
#ifdef NON_MATCHING
void BtlMemberMotion11(BtlObj *o)
{
    int script;

    switch (o->phase) {
    case 0:
        if (o->attr & HIT_OTHER_SIDE) {
            script = g_btl_models[o->kind].hit;
        } else {
            script = g_btl_member_scripts[SCRIPT_HIT
                                          + o->kind * MEMBER_SCRIPT_MODEL
                                          + o->actor->script_pick
                                                * MEMBER_SCRIPT_PICK];
        }
        BtlObjSetScript(o, (BtlSeqStep *)o->scripts[script]);
        o->timer = HIT_HOLD;
        o->phase++;
        break;
    case 1:
        if (o->timer != 0) {
            break;
        }
        if ((o->actor->flags & ACTOR_FLINCHED) == 0) {
            if (o->attr & HIT_OTHER_SIDE) {
                script = g_btl_models[o->kind].spawn;
            } else {
                script = g_btl_member_scripts[SCRIPT_STAND
                                              + o->kind * MEMBER_SCRIPT_MODEL
                                              + o->actor->script_pick
                                                    * MEMBER_SCRIPT_PICK];
            }
            BtlObjSetScript(o, (BtlSeqStep *)o->scripts[script]);
        }
        o->motion = 0;
        o->phase = 0;
        break;
    }
}
#else
INCLUDE_ASM("btlp/nonmatchings/hitmotion", BtlMemberMotion11);
#endif
