/* Persona 1 (JP) - one move whose effect stands on the fighter it is aimed at.
 * BTLP only.
 *   0x800BCF44 BtlFxStart6D
 *   0x800BCF6C BtlFxStep6D
 *
 * A start handler out of g_btl_spell_fx, and the plainest body the table has:
 * one record on the target, arriving at once. Twenty-six moves are drawn this
 * way and each has an entry point of its own.
 *
 * The step is not shared at all. Once the record's own script has run out it
 * opens sixty-four copies of itself on the same spot, one every eighth of a
 * turn round the wave tables and each a quarter of a frame later than the one
 * before, and sinks them a little toward the camera. Each copy then walks
 * outward along its own angle for thirty frames and frees itself, so the ring
 * opens out of the fighter rather than being drawn as one piece.
 *
 * The head waits out those thirty frames hidden, and on the frame they are up
 * it puts every party member it did not stand on back to full health - the
 * ones still in the fight, not already spent and not held by the timed ward -
 * records what each one had lost so the number pops up over it, and starts
 * each one on the heal reaction.
 */
#include <decomp/types.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/object.h>
#include <persona/btlp/round.h>
#include <persona/btlp/sound.h>
#include <persona/btlp/spellfx.h>

/* Copies in the ring, and which of the staged script tables they come out
   of. Sixty-four of them over the wave tables' whole turn is an eighth of a
   turn each. */
#define FX_6D_COPIES 0x40
#define FX_6D_TABLE  1
#define FX_6D_SPIN   3

/* What a copy is given on top of what BtlObjAlloc left - hidden until its own
   stagger is up, and the pair every record that carries a standing
   displacement takes - and how far toward the camera it is sunk. */
#define FX_6D_ATTR (BTL_OBJ_HIDDEN | 0xC0000)
#define FX_6D_SINK 0x180000

/* How the copies are staggered, how far one walks out in a frame, and how
   long it lives. */
#define FX_6D_STAGGER 4
#define FX_6D_OUT     2
#define FX_6D_LIFE    0x1E

/* What the head waits out while the ring is open. */
#define FX_6D_WAIT 0x1E

/* The bank and the sequence the heal is played on, and the reaction a healed
   fighter is put on. */
#define FX_6D_SE_BANK 2
#define FX_6D_SE      0xB
#define FX_6D_MOTION  0xF

BtlObj *BtlFxStart6D(void)
{
    return BtlOpenFxObj2(g_btl_fx_target, FX_TIMER);
}

void BtlFxStep6D(BtlObj *o)
{
    BtlObj *copy;
    BtlObj *after;
    int     stagger;
    int     i;

    if (o->mark_num == 0) {
        switch (o->phase) {
        case 0:
            if (o->timer != 0) {
                break;
            }
            o->attr &= ~(BTL_OBJ_HIDDEN | BTL_OBJ_STATIC);
            o->phase++;
            break;
        case 1:
            if ((o->attr & BTL_OBJ_ANIMATING) != 0) {
                break;
            }
            i = FX_6D_COPIES - 1;
            g_btl_fx_def.scripts =
                ((const u_long ***)g_btl_unused_gfx)[FX_6D_TABLE];
            after = 0;
            for (; i >= 0; i--) {
                copy = BtlObjAlloc(&g_btl_fx_def, FX_OBJ_GROUP, after,
                                   FX_OBJ_DRAW, 0, &o->x,
                                   FX_OBJ_CD, FX_OBJ_CE);
                copy->x2       = 0;
                copy->y2       = 0;
                copy->attached  = after;
                copy->z        -= FX_6D_SINK;
                copy->attr     |= FX_6D_ATTR;
                /* Load-bearing: the count lands in the local before the
                   remainder is taken of it, and that is what puts the chain's
                   own assignment in front of the division's arithmetic. */
                stagger         = i;
                stagger        %= FX_6D_STAGGER;
                after           = copy;
                after->timer    = stagger;
                after->mark_num = FX_COPY_MARK;
                after->motion   = 2;
                after->kind     = o->kind;
                after->angle    = i << FX_6D_SPIN;
            }
            o->timer = FX_6D_WAIT;
            o->attr |= BTL_OBJ_HIDDEN;
            o->phase++;
            break;
        case 2:
            if (o->timer != 0) {
                break;
            }
            for (i = 0; i < BTL_PARTY; i++) {
                if (g_btl_actors[i].c.key != 0
                    && (signed char)g_btl_actors[i].c.status != BTL_STATUS_DOWN
                    && (g_btl_actors[i].flags & BTL_ACTOR_OUT) == 0
                    && i != g_btl_fx_target
                    && (signed char)g_btl_actors[i].c.status
                           != BTL_STATUS_NOINPUT
                    && (g_btl_actors[i].flags & BTL_ACTOR_TIMED_A) == 0) {
                    BtlSePlay(FX_6D_SE_BANK, FX_6D_SE);
                    g_btl_actors[i].hit_amount =
                        g_btl_actors[i].c.hp_max - g_btl_actors[i].c.hp;
                    g_btl_actors[i].c.hp = g_btl_actors[i].c.hp_max;
                    g_btl_actors[i].obj->motion = FX_6D_MOTION;
                    g_btl_actors[i].obj->phase  = 0;
                }
            }
            g_btl_actors[g_btl_actor_turn].unkD0++;
            o->motion = 0;
            o->phase  = 0;
            break;
        default:
            break;
        }
        return;
    }
    switch (o->phase) {
    case 0:
        if (o->timer != 0) {
            break;
        }
        o->age   = 0;
        o->attr &= ~BTL_OBJ_HIDDEN;
        o->phase++;
        break;
    case 1:
        o->x2 += g_btl_wave_sin[o->angle] << FX_6D_OUT;
        o->y2 += g_btl_wave_cos[o->angle] << FX_6D_OUT;
        o->shift_x = o->x2;
        o->shift   = o->y2;
        if (o->age >= FX_6D_LIFE) {
            BtlObjFree(o);
        }
        break;
    default:
        break;
    }
}
