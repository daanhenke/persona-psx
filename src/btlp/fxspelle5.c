/* Persona 1 (JP) - the move that sends the enemies running and leaves a coin
 * behind.  BTLP only.
 *   0x800BF780 BtlFxStartE5  0x800BF7F8 BtlFxStepE5
 *
 * The start handler puts one record on the middle of the field, taking the
 * position out of the data the way move 0xA2's does.
 *
 * The step handler waits for its script, holds a second, and then walks every
 * enemy still standing off the field: each is put on the leaving motion and
 * given twenty-four frames and a twenty-fourth of its own distance from the
 * middle, so it reaches the edge as its count runs out, each starting a little
 * later than the one before. The party is
 * handed item 0x21 for the trouble. A fight that may not be run from skips all
 * of that and only holds. The record then darkens and puts itself back on
 * phase nought.
 */
#include <decomp/types.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/object.h>
#include <persona/btlp/round.h>
#include <persona/btlp/spellfx.h>

/* How long each of the two waits is, in frames. */
#define FX_E5_HOLD 0x3C

/* What an enemy on its way out is given: the leaving motion, how many frames
   it has, and how much later each one starts. The step is its own distance
   from the middle divided by the frames it has, so it arrives as its count
   runs out. */
#define FX_E5_MOTION  0x13
#define FX_E5_FRAMES  0x18
#define FX_E5_STAGGER 4

/* What the party is handed for it. */
#define FX_E5_GIFT 0x21

/* How fast the record darkens at the end. */
#define FX_E5_FADE 8

BtlObj *BtlFxStartE5(void)
{
    BtlObj *o;

    g_btl_fx_def.scripts = ((const u_long ***)g_btl_unused_gfx)[0];
    o = BtlObjAlloc(&g_btl_fx_def, FX_OBJ_GROUP, 0, FX_OBJ_DRAW, 0,
                    g_btl_fx_origin, FX_OBJ_CD, FX_OBJ_CE);
    o->mark_num = FX_MARK_HEAD;
    o->attr |= BTL_OBJ_NO_SHADOW;
    return o;
}

void BtlFxStepE5(BtlObj *o)
{
    BtlObj *t;
    int     slot;
    int     n;

    switch (o->phase) {
    case 0:
        if ((o->attr & BTL_OBJ_ANIMATING) != 0) {
            break;
        }
        o->timer = FX_E5_HOLD;
        o->phase++;
        break;
    case 1:
        if (o->timer != 0) {
            break;
        }
        if (g_btl_no_escape == 0) {
            slot = BTL_PARTY;
            n = 0;
            for (; slot < BTL_ACTORS; slot++) {
                if (g_btl_actors[slot].c.key == 0) {
                    continue;
                }
                if ((signed char)g_btl_actors[slot].c.status
                        == BTL_STATUS_DOWN) {
                    continue;
                }
                if ((g_btl_actors[slot].flags & BTL_ACTOR_OUT) != 0) {
                    continue;
                }
                t = g_btl_actors[slot].obj;
                t->step_x = -(t->x / FX_E5_FRAMES);
                t->step_y = -(t->y / FX_E5_FRAMES);
                t->steps = FX_E5_FRAMES;
                t->timer = n * FX_E5_STAGGER;
                n++;
                t->motion = FX_E5_MOTION;
            }
            BtlGiveItem(FX_E5_GIFT);
        }
        o->timer = FX_E5_HOLD;
        o->phase++;
        break;
    case 2:
        if (o->timer != 0) {
            break;
        }
        o->rgb_to[0] = 0;
        o->rgb_to[1] = 0;
        o->rgb_to[2] = 0;
        o->fade = FX_E5_FADE;
        o->phase++;
        break;
    case 3:
        if (o->rgb[0] != 0) {
            break;
        }
        o->motion = 0;
        o->phase = 0;
        break;
    default:
        break;
    }
}
