/* Persona 1 (JP) - the move that turns the fighter aimed at to stone.
 * BTLP only.
 *   0x800BE3C8 BtlFxStep86
 *
 * One frame of work, and three ways out of it. A fighter whose flags say the
 * hit does not reach it gets the ringing sound and has its whole palette
 * written white instead - the fading list is told to walk it back - and the
 * move ends there. A fighter already carrying the ailment is left with it and
 * only put back on its feet. Anything else has the ailment inflicted, and if
 * it takes, the fighter is put on the stiffening motion and the move holds
 * half a second before it lets go.
 */
#include <decomp/types.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/object.h>
#include <persona/btlp/round.h>
#include <persona/btlp/sound.h>
#include <persona/btlp/spellfx.h>

/* The ailment this move leaves, and the motion a fighter takes it on. */
#define FX_86_STATUS 0xE
#define FX_86_MOTION 7

/* The flags that say the hit does not reach: the fighter is warded, and the
   one that says it is out of the fight altogether. */
#define FX_86_WARDED 0x1880
#define FX_86_GONE   0x40

/* The two sounds, on the effects slot. */
#define FX_86_SLOT  2
#define FX_86_WARD  0xA
#define FX_86_LAND  5

/* A palette is 256 entries; entry nought is not written. */
#define FX_86_CLUT  0x100
#define FX_86_WHITE 0xFFFF

/* How long the move holds once the ailment lands. */
#define FX_86_HOLD 0x1E

extern u_short  g_btl_clut_fading;
extern u_short *g_btl_actor_clut;

extern void func_80097484(BtlActor *a);

void BtlFxStep86(BtlObj *o)
{
    BtlActor *t;
    int       i;

    t = &g_btl_actors[g_btl_fx_target];
    switch (o->phase) {
    case 0:
        if ((o->attr & BTL_OBJ_ANIMATING) != 0) {
            return;
        }
        o->attr |= BTL_OBJ_HIDDEN;
        if ((t->flags & FX_86_WARDED) != 0) {
            BtlSePlay(FX_86_SLOT, FX_86_WARD);
            g_btl_clut_fading |= 1 << g_btl_hit_slot;
            i = 1;
            do {
                g_btl_actor_clut[g_btl_hit_slot * FX_86_CLUT + i] =
                    FX_86_WHITE;
                i++;
            } while (i < FX_86_CLUT);
            o->motion = 0;
            o->phase = 0;
            return;
        }
        if ((g_btl_actors[g_btl_fx_target].flags & FX_86_GONE) != 0) {
            o->motion = 0;
            o->phase = 0;
            return;
        }
        if ((signed char)g_btl_actors[g_btl_fx_target].c.status
                == FX_86_STATUS) {
            func_80097484(&g_btl_actors[g_btl_fx_target]);
            g_btl_actors[g_btl_fx_target].unk84 = 0;
            g_btl_actors[g_btl_fx_target].obj->motion = FX_86_MOTION;
            g_btl_actors[g_btl_fx_target].obj->phase = 0;
            o->timer = FX_86_HOLD;
        } else if (BtlInflictStatus(&g_btl_actors[g_btl_fx_target],
                                    FX_86_STATUS) != 0) {
            g_btl_actors[g_btl_fx_target].unk84 = 0;
            g_btl_actors[g_btl_fx_target].obj->motion = FX_86_MOTION;
            g_btl_actors[g_btl_fx_target].obj->phase = 0;
            BtlSePlay(FX_86_SLOT, FX_86_LAND);
            o->timer = FX_86_HOLD;
        }
        o->phase++;
        return;
    case 1:
        if (o->timer != 0) {
            return;
        }
        o->motion = 0;
        o->phase = 0;
        return;
    default:
        return;
    }
}
