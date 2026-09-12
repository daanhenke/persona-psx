/* Persona 1 (JP) - the move that stops the other side running.  BTLP only.
 *   0x800BF4F8 BtlFxStartA2  0x800BF55C BtlFxStepA2
 *
 * One record on the middle of the field, and the only start handler in the
 * table that takes its position out of the data rather than working one out.
 *
 * The step handler waits for the record's script to play out, throws the arena
 * yellow, holds for a second, and then locks the other side out of running for
 * three rounds - the enemy AI's own count when a party member cast it, and the
 * escape menu's when an enemy did. It puts the record back on phase nought
 * afterwards rather than finishing it, so the effect stands until whatever
 * opened it takes it away.
 */
#include <decomp/types.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/object.h>
#include <persona/btlp/round.h>
#include <persona/btlp/spellfx.h>

/* The colour the arena is thrown, and how fast. */
#define FX_A2_R    0xFF
#define FX_A2_G    0xFF
#define FX_A2_B    0
#define FX_A2_FADE 4

/* How long the record holds before the lock goes on, and how many rounds the
   lock lasts. */
#define FX_A2_HOLD 0x3C
#define FX_A2_LOCK 3

BtlObj *BtlFxStartA2(void)
{
    g_btl_fx_def.scripts = ((const u_long ***)g_btl_unused_gfx)[0];
    return BtlObjAlloc(&g_btl_fx_def, FX_OBJ_GROUP, 0, FX_OBJ_DRAW, 0,
                       g_btl_fx_middle, FX_OBJ_CD, FX_OBJ_CE);
}

void BtlFxStepA2(BtlObj *o)
{
    switch (o->phase) {
    case 0:
        if ((o->attr & BTL_OBJ_ANIMATING) != 0) {
            break;
        }
        o->attr |= BTL_OBJ_HIDDEN;
        g_btl_arena_rgb[0] = FX_A2_R;
        g_btl_arena_rgb[1] = FX_A2_G;
        g_btl_arena_rgb[2] = FX_A2_B;
        g_btl_arena_fade = FX_A2_FADE;
        o->timer = FX_A2_HOLD;
        o->phase++;
        break;
    case 1:
        if (o->timer != 0) {
            break;
        }
        if (g_btl_actor_turn < BTL_PARTY) {
            g_btl_no_flee = FX_A2_LOCK;
            o->motion = 0;
        } else {
            g_btl_party_no_flee = FX_A2_LOCK;
            o->motion = 0;
        }
        o->phase = 0;
        break;
    default:
        break;
    }
}
