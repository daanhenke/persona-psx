/* Persona 1 (JP) - the step an effect record gets while the talk holds it.
 * BTLP only.
 *   0x800BFDAC BtlActorObjPos  0x800BFE14 BtlTalkFxObjTick
 *
 * BtlActorObjPos copies where a fighter's record stands into a position.
 * Nothing calls it - there is no jal to it and no word holding its address
 * anywhere in the overlay.
 *
 * BtlTalkFxObjTick is what BtlTickEffects runs in place of BtlFxObjTick on a
 * record carrying BTL_OBJ_ATTR_2000, and it goes by the record's kind. Six
 * kinds wait out their timer hidden and static and are then shown; on the
 * phase after that they take the same test as any kind not named here, which
 * frees a record once its script has stopped animating. Kind 0xBB drifts round
 * the wave tables by its angle, and is freed when its timer runs out.
 */
#include <decomp/types.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/object.h>

/* The drift is a quarter-step of the wave tables' 16.16 per frame. */
#define TALK_FX_DRIFT 4

void BtlActorObjPos(long *pos, int slot)
{
    pos[0] = g_btl_actors[slot].obj->x;
    pos[1] = g_btl_actors[slot].obj->y;
    pos[2] = g_btl_actors[slot].obj->z;
}

void BtlTalkFxObjTick(BtlObj *o)
{
    switch (o->kind) {
    case 0xAF:
    case 0xBD:
    case 0xCA:
    case 0xD3:
    case 0xD4:
    case 0xDE:
        switch (o->phase) {
        case 0:
            if (o->timer == 0) {
                o->attr &= ~(BTL_OBJ_HIDDEN | BTL_OBJ_STATIC);
                o->phase++;
            }
            break;
        case 1:
            goto other;
        }
        break;
    case 0xBB:
        if (o->phase == 0) {
            o->shift_x += g_btl_wave_sin[o->angle] * TALK_FX_DRIFT;
            o->shift += g_btl_wave_cos[o->angle] * TALK_FX_DRIFT;
            if (o->timer == 0) {
                BtlObjFree(o);
            }
        }
        break;
    default:
    other:
        if ((o->attr & BTL_OBJ_ANIMATING) == 0) {
            BtlObjFree(o);
        }
        break;
    }
}
