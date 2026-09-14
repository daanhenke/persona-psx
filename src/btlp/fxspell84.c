/* Persona 1 (JP) - the move whose effect starts above the fighter and falls.
 * BTLP only.
 *   0x800BDF60 BtlFxStart84
 *   0x800BDFB4 BtlFxStep84
 *
 * A start handler out of g_btl_spell_fx, shared by moves 0x84 and 0x85. The
 * record is the ordinary one on the fighter aimed at, and then three things are
 * written on it by hand: a count for the step handler to work down, four units
 * of height so the artwork begins above the fighter and drops onto it, and the
 * hidden bit cleared so it is drawn from this frame rather than the next.
 *
 * The step handler drops it: each frame it falls a unit and swings toward and
 * away from the camera on the wave tables, and when the count is spent it is
 * let go of its own script and put on the second one. Once that has played out
 * the fighter is turned to stone, the way BtlFxStep86 does it - the same three
 * ways out - except that the two moves that share the handler disagree about
 * which fighters they reach: 0x84 passes over a fighter whose palette starts
 * past the first entry and 0x85 only reaches those.
 */
#include <decomp/types.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/clut.h>
#include <persona/btlp/object.h>
#include <persona/btlp/round.h>
#include <persona/btlp/sound.h>
#include <persona/btlp/spellfx.h>
#include <persona/btlp/status.h>

/* What the step handler counts down, and how far above the fighter the record
   starts - 16.16, so four whole units. */
#define FX_84_COUNT 0x40
#define FX_84_RISE  0x400000

BtlObj *BtlFxStart84(void)
{
    BtlObj *obj;

    obj = BtlOpenFxObj2(g_btl_fx_target, FX_TIMER);
    obj->steps = FX_84_COUNT;
    obj->y    -= FX_84_RISE;
    obj->attr &= ~BTL_OBJ_HIDDEN;
    return obj;
}

/* How far the record falls in a frame, how fast it swings, and how far it
   swings - the wave tables are 12.12 and the reading is doubled. */
#define FX_84_FALL 0x10000
#define FX_84_SPIN 4
#define FX_84_SWAY 1

/* The two moves that share the handler, and which of the record's scripts the
   landing is played on. */
#define FX_84_MOVE 0x84
#define FX_85_MOVE 0x85
#define FX_84_TABLE 1

/* The ailment this move leaves, and the motion a fighter takes it on. */
#define FX_84_STATUS 0xE
#define FX_84_MOTION 7

/* The flags that say the hit does not reach: the fighter is warded, and the
   one that says it is out of the fight altogether. */
#define FX_84_WARDED 0x1880
#define FX_84_GONE   0x40

/* The two sounds, on the effects slot. */
#define FX_84_SLOT 2
#define FX_84_WARD 0xA
#define FX_84_LAND 5

/* A palette is 256 entries; entry nought is not written. */
#define FX_84_CLUT  0x100
#define FX_84_WHITE 0xFFFF

/* How long the move holds once the ailment lands. */
#define FX_84_HOLD 0x1E

void BtlFxStep84(BtlObj *o)
{
    BtlActor *t;
    int       first;
    int       i;

    t = &g_btl_actors[g_btl_fx_target];
    switch (o->phase) {
    case 0:
        o->z  = g_btl_wave_sin[(o->steps << FX_84_SPIN) & BTL_WAVE_MASK]
                    * (1 << FX_84_SWAY) + o->z2;
        o->y += FX_84_FALL;
        o->steps--;
        if (o->steps > 0) {
            return;
        }
        o->attr &= ~BTL_OBJ_STATIC;
        o->phase++;
        return;
    case 1:
        if ((o->attr & BTL_OBJ_ANIMATING) != 0) {
            return;
        }
        BtlObjSetScript(o, (BtlSeqStep *)o->scripts[FX_84_TABLE]);
        o->phase++;
        return;
    case 2:
        if ((o->attr & BTL_OBJ_ANIMATING) != 0) {
            return;
        }
        o->attr |= BTL_OBJ_HIDDEN;
        first = BtlActorClutFirst(t);
        if ((t->flags & FX_84_WARDED) != 0) {
            BtlSePlay(FX_84_SLOT, FX_84_WARD);
            g_btl_clut_fading |= 1 << g_btl_hit_slot;
            i = 1;
            do {
                g_btl_actor_clut[g_btl_hit_slot * FX_84_CLUT + i] =
                    FX_84_WHITE;
                i++;
            } while (i < FX_84_CLUT);
            o->motion = 0;
            o->phase = 0;
            return;
        }
        if ((g_btl_actors[g_btl_fx_target].flags & FX_84_GONE) != 0) {
            o->motion = 0;
            o->phase = 0;
            return;
        }
        switch (g_btl_fx_move) {
        case FX_84_MOVE:
            if (first != 0) {
                o->motion = 0;
                o->phase = 0;
                return;
            }
            break;
        case FX_85_MOVE:
            if (first == 0) {
                o->motion = 0;
                o->phase = 0;
                return;
            }
            break;
        default:
            break;
        }
        if ((signed char)g_btl_actors[g_btl_fx_target].c.status
                == FX_84_STATUS) {
            BtlDrainLevel(&g_btl_actors[g_btl_fx_target]);
            g_btl_actors[g_btl_fx_target].hit_amount = 0;
            g_btl_actors[g_btl_fx_target].obj->motion = FX_84_MOTION;
            g_btl_actors[g_btl_fx_target].obj->phase = 0;
            o->timer = FX_84_HOLD;
        } else if (BtlInflictStatus(&g_btl_actors[g_btl_fx_target],
                                    FX_84_STATUS) != 0) {
            g_btl_actors[g_btl_fx_target].hit_amount = 0;
            g_btl_actors[g_btl_fx_target].obj->motion = FX_84_MOTION;
            g_btl_actors[g_btl_fx_target].obj->phase = 0;
            BtlSePlay(FX_84_SLOT, FX_84_LAND);
            o->timer = FX_84_HOLD;
        }
        o->phase++;
        return;
    case 3:
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
