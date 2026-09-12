/* Persona 1 (JP) - two of the motions a field record takes.  BTLP only.
 *   0x800B07F4 BtlMemberMotion12  0x800B08FC BtlMemberMotion13
 *
 * The last two entries of g_btl_member_motion, which is the table the group
 * the party stands in is ticked through - so they are named for the motion
 * they answer to rather than for what they do, the way the effect table's
 * handlers are.
 *
 * 0x12 swings the record round the point it was put at. The angle steps eight
 * places of the wave tables a frame and the radius is mark_num, opened out two
 * at a time until it reaches 0x40 and then closed again; the record is freed
 * the moment it is back at nothing. Which way it is going is one attribute
 * bit, turned over rather than tested, so the whole of it is that one bit and
 * the one byte.
 *
 * 0x13 is a straight glide: the step pair is added to the position for as many
 * frames as `steps` has left, and on the last of them the record is put back
 * on motion zero with its second copy of the position brought up to where it
 * finished - so whatever moves it next starts from there rather than from
 * where it set off. The attack lines are refreshed at the same point, which is
 * what says the glide is a fighter changing places.
 */
#include <decomp/types.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/object.h>
#include <persona/btlp/round.h>

/* Which way the radius is going. Motion 0x12 turns it over at the top rather
   than keeping a phase, so it is never read anywhere else. */
#define SWING_CLOSING 0x100

/* How wide the swing opens, what it opens by in a frame, and how far the
   angle moves. The radius is a quarter of mark_num. */
#define SWING_WIDEST 0x40
#define SWING_STEP   2
#define SWING_TURN   8
#define SWING_SHIFT  2

void BtlMemberMotion12(BtlObj *o)
{
    if ((o->attr & SWING_CLOSING) == 0) {
        o->mark_num += SWING_STEP;
        if (o->mark_num == SWING_WIDEST) {
            o->attr ^= SWING_CLOSING;
        }
    } else {
        o->mark_num -= SWING_STEP;
        if (o->mark_num == 0) {
            BtlObjFree(o);
        }
    }

    /* Both the radius and the angle are taken again for the second
       coordinate; held in locals, the pair comes out a register short. */
    o->x = (o->mark_num >> SWING_SHIFT)
           * g_btl_wave_sin[o->angle & BTL_WAVE_MASK] + o->x2;
    o->y = (o->mark_num >> SWING_SHIFT)
           * g_btl_wave_cos[o->angle & BTL_WAVE_MASK] + o->y2;
    o->angle += SWING_TURN;
}

void BtlMemberMotion13(BtlObj *o)
{
    if (o->phase == 0) {
        o->x += o->step_x;
        o->y += o->step_y;
        if (--o->steps == 0) {
            BtlRefreshAttacks();
            o->motion = 0;
            o->phase = 0;
            o->x2 = o->x;
            o->y2 = o->y;
        }
    }
}
