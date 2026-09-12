/* Persona 1 (JP) - the motion a beaten demon leaves the field on.  BTLP only.
 *   0x800B6250 BtlObjTickVanish
 *
 * One of the handlers the motion table dispatches on, run once a frame while
 * it is the object's motion. It carries the object along by its own step,
 * shrinks it - and the shadow with it - until there is nothing left, and when
 * the frame count runs out stops the motion and takes the fighter's slot away
 * by clearing the Char key nobody else can be in that slot without.
 *
 * Nothing happens at all until the object is at the start of its motion and
 * its timer has run down, so a script can hold the shrink off for as long as
 * it likes before letting this have it.
 *
 * The scale is taken to zero rather than allowed past it, and the position is
 * copied into the second pair at the end, which is what leaves the record
 * standing where it stopped rather than where the motion began.
 */
#include <decomp/types.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/object.h>

/* Set as the object starts vanishing, the same bit the whitening sets. */
#define BTL_OBJ_ATTR_4000 0x4000

/* How much of the scale goes each frame; unity is 0x100. */
#define VANISH_STEP 0xAA

void BtlObjTickVanish(BtlObj *o)
{
    if (o->phase != 0) {
        return;
    }
    if (o->timer != 0) {
        return;
    }

    o->attr |= BTL_OBJ_ATTR_4000;
    o->x += o->step_x;
    o->y += o->step_y;

    o->scale_x -= VANISH_STEP;
    if (o->scale_x < 0) {
        o->scale_x = 0;
    }
    /* Each of the four reaches through the object again; held in locals the
       two scales come out in the wrong registers. */
    o->scale_y = o->scale_x;
    o->shadow->scale_x = o->scale_x;
    o->shadow->scale_y = o->scale_y;

    if (--o->steps != 0) {
        return;
    }

    o->motion = 0;
    o->phase = 0;
    o->x2 = o->x;
    o->y2 = o->y;
    o->actor->c.key = 0;
}
