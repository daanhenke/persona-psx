/* Persona 1 (JP) - the move whose effect is scattered around the fighter it
 * is aimed at.  BTLP only.
 *   0x800B7354 BtlFxStartRing
 *
 * A start handler out of g_btl_spell_fx. Six records rather than one, each
 * standing a few pixels off the fighter's own position and each starting four
 * frames after the one before it, so the effect breaks up rather than
 * arriving whole. The fighter itself is put on the same blue the plainer
 * handlers use.
 *
 * The records are made from the last offset back to the first, and the last
 * one made - the one standing exactly on the fighter - is the head of the
 * chain and the one answered.
 */
#include <decomp/types.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/object.h>
#include <persona/btlp/round.h>
#include <persona/btlp/spellfx.h>

/* The blue the fighter under the effect is walked to, and how fast. */
#define FX_RING_R    0
#define FX_RING_G    0x80
#define FX_RING_B    0xFF
#define FX_RING_FADE 1

/* Records in the scatter, and how much later each one starts. */
#define FX_RING       6
#define FX_RING_STEP  4

/* What a scattered record starts as: the plain effect attributes and the bit
   at 0x40000 beside them. */
#define FX_RING_ATTR (FX_OBJ_ATTR | 0x40000)

/* Where each record stands relative to the fighter, in 16.16. */
extern long g_btl_fx_ring[][2];

BtlObj *BtlFxStartRing(void)
{
    BtlObj *o;
    BtlObj *after;
    BtlObj *head;
    long    pos[3];
    long    base[3];
    int     i;
    int     slot;

    slot = g_btl_actors[g_btl_actor_turn].order;
    base[0] = g_btl_actors[slot].obj->x;
    base[1] = g_btl_actors[slot].obj->y;
    base[2] = 0;

    g_btl_actors[slot].obj->rgb_to[0] = FX_RING_R;
    g_btl_actors[slot].obj->rgb_to[1] = FX_RING_G;
    g_btl_actors[slot].obj->rgb_to[2] = FX_RING_B;
    g_btl_actors[slot].obj->fade      = FX_RING_FADE;

    i = FX_RING - 1;
    g_btl_fx_def.scripts = ((const u_long ***)g_btl_unused_gfx)[0];
    after = 0;
    for (; i >= 0; i--)
    {
        pos[0] = base[0] + g_btl_fx_ring[i][0];
        pos[1] = base[1] + g_btl_fx_ring[i][1];
        pos[2] = 0;
        o = BtlObjAlloc(&g_btl_fx_def, FX_OBJ_GROUP, after, FX_OBJ_DRAW, 0,
                        pos, FX_OBJ_CD, FX_OBJ_CE);
        o->attr     = FX_RING_ATTR;
        o->attached = after;
        after       = o;
        o->mark_num = i;
        o->timer    = i * FX_RING_STEP;
        if (i == 0)
        {
            head = o;
        }
    }
    return head;
}
