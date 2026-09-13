/* Persona 1 (JP) - two step handlers nothing reaches.  BTLP only.
 *   0x800C0740 BtlFxMotionUnused  0x800C0840 BtlFxStepOrbitUnused
 *
 * BtlFxMotionUnused is BtlFxStep01's per-move motion on its own, without the
 * fade to black move 0x1B takes there. BtlFxStepOrbitUnused swings its record
 * round a point on the wave tables while the record grows to full size, holds
 * for a second and shrinks away. No record in g_btl_spell_fx points at
 * either, nothing calls them, and no word in BTLP.BIN holds their addresses -
 * the same way BtlFxStepUnused and BtlFxStepUnused2 are left behind.
 */
#include <decomp/types.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/object.h>
#include <persona/btlp/round.h>
#include <persona/btlp/spellfx.h>

/* The orbit: how fast the record grows and shrinks, the size it stops
   growing at and the one it is let go below, how long it holds, how fast it
   goes round and how it is tilted, and how far out and back it stands. */
#define FX_ORBIT_GROW   8
#define FX_ORBIT_FULL   0x1000
#define FX_ORBIT_GONE   0x11
#define FX_ORBIT_HOLD   60
#define FX_ORBIT_SPEED  8
#define FX_ORBIT_TILT   0x200
#define FX_ORBIT_RADIUS 92
#define FX_ORBIT_BACK   0x500000

void BtlFxMotionUnused(BtlObj *o)
{
    switch (o->kind) {
    case FX_STEP_THROW:
    case FX_STEP_THROW2:
        if (o->steps-- > 0) {
            o->y += o->step_y;
        }
        break;
    case FX_STEP_RISE:
    case FX_STEP_RISE2:
        o->y += o->step_y;
        o->step_y += g_btl_actor_turn < BTL_PARTY ? -FX_STEP_FALL : FX_STEP_FALL;
        break;
    case FX_STEP_SLIDE:
        o->y += g_btl_actor_turn < BTL_PARTY ? -FX_STEP_SLIDE_BY
                                             : FX_STEP_SLIDE_BY;
        break;
    }
    if ((u_short)(o->kind - FX_STEP_TINT_FIRST) < FX_STEP_TINT_MOVES) {
        g_btl_arena_rgb[1] = FX_STEP_TINT_G;
        g_btl_arena_rgb[2] = FX_STEP_TINT_B;
        g_btl_arena_rgb[0] = FX_STEP_TINT_R;
        g_btl_arena_fade = FX_STEP_TINT_FADE;
    }
}

void BtlFxStepOrbitUnused(BtlObj *o)
{
    long c;

    switch (o->phase) {
    case 1:
        o->scale_x += o->scale_x / 8 + FX_ORBIT_GROW;
        o->scale_y = o->scale_x;
        if (o->scale_x >= FX_ORBIT_FULL) {
            o->scale_x = FX_ORBIT_FULL;
            o->scale_y = FX_ORBIT_FULL;
            o->timer = FX_ORBIT_HOLD;
            o->phase++;
        }
        break;
    case 2:
        if (o->timer == 0) {
            o->scale_x -= o->scale_x / 8;
            if (o->scale_x < FX_ORBIT_GONE) {
                o->attr = (o->attr | BTL_OBJ_STATIC) & ~BTL_OBJ_ANIMATING;
                o->phase++;
            }
        }
        break;
    }
    o->angle = (o->angle + FX_ORBIT_SPEED) & BTL_WAVE_MASK;
    o->rot.vy = o->angle * 8 + FX_ORBIT_TILT;
    o->x = g_btl_wave_sin[o->angle] * FX_ORBIT_RADIUS;
    c = g_btl_wave_cos[o->angle] * FX_ORBIT_RADIUS;
    if (g_btl_actor_turn < BTL_PARTY) {
        o->y = c - FX_ORBIT_BACK;
    } else {
        o->y = c + FX_ORBIT_BACK;
    }
}
