/* Persona 1 (JP) - a move whose effect is drawn and moving at once, and the
 * drain it lands.  BTLP only.
 *   0x800BE634 BtlFxStart87  0x800BE670 BtlFxStep87
 *
 * A start handler out of g_btl_spell_fx. The record is the ordinary one on the
 * fighter aimed at, with the two bits an effect normally opens holding - hidden
 * and static - cleared straight away, so the artwork is drawn and its script
 * runs from the frame it is taken rather than from the next. Move 0x86 has the
 * same handler written out again.
 *
 * BtlFxStep87 waits for the artwork to finish. A fighter the hit does not reach
 * gets BtlFxStep86's ringing and white palette and the move ends; one out of
 * the fight ends it quietly. Anything else loses experience: a roll of 0 to 7
 * times the gap between the caster's fourth stat and the fighter's, plus the
 * caster's level, held between nothing and 2000. It comes off the experience
 * toward the next level and the total and is added to what the level still
 * wants - or, where there is not that much toward the next level, a whole level
 * is drained instead. The fighter is put on its hit motion, and the move holds
 * FX_87_HOLD frames.
 */
#include <decomp/types.h>
#include <rand.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/object.h>
#include <persona/btlp/round.h>
#include <persona/btlp/sound.h>
#include <persona/btlp/spellfx.h>

/* The flags that say the hit does not reach: warded, or out of the fight. */
#define FX_87_WARDED 0x1880
#define FX_87_GONE   0x40

/* The ringing sound, on the effects slot. */
#define FX_87_SLOT 2
#define FX_87_WARD 0xA

/* A palette is 256 entries; entry nought is not written. */
#define FX_87_CLUT  0x100
#define FX_87_WHITE 0xFFFF

/* The roll, the most a drain takes, the motion the fighter is put on, and how
   long the move holds. */
#define FX_87_ROLL   7
#define FX_87_MOST   2000
#define FX_87_MOTION 7
#define FX_87_HOLD   0x20

extern u_short  g_btl_clut_fading;
extern u_short *g_btl_actor_clut;

BtlObj *BtlFxStart87(void)
{
    BtlObj *obj;

    obj = BtlOpenFxObj2(g_btl_fx_target, FX_TIMER);
    obj->attr &= ~(BTL_OBJ_HIDDEN | BTL_OBJ_STATIC);
    return obj;
}

void BtlFxStep87(BtlObj *o)
{
    BtlActor *t;
    BtlActor *by;
    int       drain;
    int       exp;
    int       i;

    t = &g_btl_actors[g_btl_fx_target];
    by = o->actor;
    switch (o->phase) {
    case 0:
        if ((o->attr & BTL_OBJ_ANIMATING) != 0) {
            return;
        }
        o->attr |= BTL_OBJ_HIDDEN;
        if ((t->flags & FX_87_WARDED) != 0) {
            BtlSePlay(FX_87_SLOT, FX_87_WARD);
            g_btl_clut_fading |= 1 << g_btl_hit_slot;
            i = 1;
            do {
                g_btl_actor_clut[g_btl_hit_slot * FX_87_CLUT + i] =
                    FX_87_WHITE;
                i++;
            } while (i < FX_87_CLUT);
            o->motion = 0;
            o->phase = 0;
            return;
        }
        if ((g_btl_actors[g_btl_fx_target].flags & FX_87_GONE) != 0) {
            o->motion = 0;
            o->phase = 0;
            return;
        }
        drain = (by->stat[4] - t->stat[4] + by->c.level) * (rand() & FX_87_ROLL);
        drain = drain < 0 ? 0 : drain > FX_87_MOST ? FX_87_MOST : drain;
        exp = t->c.unk14;
        t->hit_amount = 0;
        t->obj->motion = FX_87_MOTION;
        t->obj->phase = 0;
        if (exp - drain >= 0) {
            t->c.unk14 -= drain;
            /* Written the other way round from the image: the scheduler
               swaps the two stores back. */
            t->c.unk10 -= drain;
            t->c.unk18 += drain;
        } else {
            BtlDrainLevel(t);
        }
        o->timer = FX_87_HOLD;
        o->phase++;
        break;
    case 1:
        if (o->timer != 0) {
            return;
        }
        o->motion = 0;
        o->phase = 0;
        break;
    }
}
