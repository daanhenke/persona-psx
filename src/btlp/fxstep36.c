/* Persona 1 (JP) - one frame of the move drawn as a turning disc and the
 * pieces going round it.  BTLP only.
 *   0x800BA4FC BtlFxStep36
 *
 * A step handler out of g_btl_spell_fx, called once a frame on each record of
 * the set. Which record it is is told by the mark: the head is the disc, and
 * it turns on its own axis a step at a time; a copy is one of the pieces going
 * round, and it is walked four units further round the wave tables each frame
 * and stood at that point of a circle ninety-two units across, over the far
 * side of the field.
 *
 * Whichever it is, once the record has stood for as long as the start handler
 * gave it, it is told to walk back to black, and the head arms the hit and the
 * copies free themselves once it has.
 */
#include <decomp/types.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/object.h>
#include <persona/btlp/round.h>
#include <persona/btlp/spellfx.h>

/* How far the disc turns each frame, and how far round the circle a piece
   moves. The wave tables run 0x200 to the turn. */
#define FX_36_SPIN 0x40
#define FX_36_STEP 4
#define FX_36_TURN 0x200

/* How far out the pieces stand, and how far past the middle of the field the
   circle they go round is. */
#define FX_36_RADIUS 92
#define FX_36_Y      0x500000

/* How long the set stands, how fast it drains, and the phase the head is left
   on once the hit is armed. */
#define FX_36_HOLD 0x40
#define FX_36_FADE 2
#define FX_36_DONE 0x80

/* The mark a piece going round carries. */
#define FX_COPY_MARK 0xFF

void BtlFxStep36(BtlObj *o)
{
    switch (o->phase) {
    case 0:
        switch (o->mark_num) {
        case FX_MARK_HEAD:
            o->rot.vz -= FX_36_SPIN;
            break;
        case FX_COPY_MARK:
            o->angle = (o->angle + FX_36_STEP) & (FX_36_TURN - 1);
            o->x = g_btl_wave_sin[o->angle] * FX_36_RADIUS;
            /* The side the circle stands on is picked inside the y statement,
               not in front of it: written as an if of its own the whole
               cosine walk moves behind the branch. */
            o->y = g_btl_wave_cos[o->angle] * FX_36_RADIUS
                   + (g_btl_actor_turn < BTL_PARTY ? -FX_36_Y : FX_36_Y);
            break;
        default:
            break;
        }
        if (o->timer != 0) {
            break;
        }
        o->timer = FX_36_HOLD;
        o->fade = FX_36_FADE;
        o->rgb_to[0] = 0;
        o->rgb_to[1] = 0;
        o->rgb_to[2] = 0;
        o->phase++;
        break;
    case 1:
        if (o->timer != 0) {
            break;
        }
        switch (o->mark_num) {
        case FX_MARK_HEAD:
            o->phase = FX_36_DONE;
            BtlArmHitChain();
            o->children = (u_char)g_btl_spell_fx[g_btl_fx_move].group;
            break;
        case FX_COPY_MARK:
            BtlObjFree(o);
            break;
        default:
            break;
        }
        break;
    default:
        BtlFxFinish01(o);
        break;
    }
}
