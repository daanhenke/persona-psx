/* Persona 1 (JP) - one frame of the ring breaking outward.  BTLP only.
 *   0x800BE9E8 BtlFxStep88
 *
 * A step handler out of g_btl_spell_fx, called once a frame on each of the
 * ring's thirty-two records. A record waits out the delay its place in the
 * ring gave it, uncovers itself, and then drifts along the step it was opened
 * with until its script has played out. The piece that is the head of the
 * chain arms the hit as it finishes; every other piece just frees itself.
 */
#include <decomp/types.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/object.h>
#include <persona/btlp/round.h>
#include <persona/btlp/spellfx.h>

/* The phase the head is left on once the hit is armed, which is what sends
   the next frame into the finish handler. */
#define FX_88_DONE 0x80

void BtlFxStep88(BtlObj *o)
{
    switch (o->phase) {
    case 0:
        if (o->timer != 0) {
            break;
        }
        o->attr &= ~(BTL_OBJ_HIDDEN | BTL_OBJ_STATIC);
        o->phase++;
        break;
    case 1:
        if ((o->attr & BTL_OBJ_ANIMATING) == 0) {
            if (o->mark_num == FX_MARK_HEAD) {
                o->attr |= BTL_OBJ_HIDDEN;
                o->phase = FX_88_DONE;
                o->children = (u_char)g_btl_spell_fx[o->kind].group;
                BtlArmHitChain();
            } else {
                BtlObjFree(o);
            }
            break;
        }
        o->x += o->step_x;
        o->y += o->step_y;
        break;
    default:
        BtlFinishMoveFx(o);
        break;
    }
}
