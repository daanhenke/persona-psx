/* Persona 1 (JP) - move 0x39's effect: a scatter on everyone it reaches.
 * BTLP only.
 *   0x800BAAA4 BtlFxStart39  0x800BABD0 BtlOpenFxScatter
 *
 * A start handler out of g_btl_spell_fx, the same walk as BtlFxStart32 with a
 * scatter where that one opens a stack: every fighter the acting record's
 * target mask reaches - in the fight, neither down nor out - gets one of its
 * own, each one's mark is counted up as it is opened, and the scatters are
 * chained one behind another through the last record of each. The last one
 * opened is given FX_MARK_HEAD and answered.
 *
 * BtlOpenFxScatter is the scatter: nine records on the fighter in the given
 * slot, sunk below its feet and drawn from the effect's three staged script
 * tables in turn. Each is pushed off the fighter along the wave tables by a
 * random fraction of a sixteenth - the angle a ninth of the way further round
 * for each record - and each arrives two frames after the one below it, from
 * the timer the caller passes. The records are chained downward, so the last
 * made is the head and the one answered.
 */
#include <decomp/types.h>
#include <rand.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/object.h>
#include <persona/btlp/round.h>
#include <persona/btlp/spellfx.h>

/* Records in a scatter, the staged tables they are drawn from, how far round
   the wave tables each stands from the next, and how late each arrives. */
#define FX_39_PIECES  9
#define FX_39_TABLES  3
#define FX_39_SPACING 0x38
#define FX_39_STEP    2
#define FX_39_SHIFT   16

/* How far under the fighter the scatter stands, and what its records carry. */
#define FX_39_DEPTH (-0x180000)
#define FX_39_ATTR  (FX_OBJ_ATTR | BTL_OBJ_SHIFT_SCREEN | 0x80000)

extern BtlObj *BtlOpenFxScatter(int slot, int timer);

BtlObj *BtlFxStart39(void)
{
    BtlObj *o;
    BtlObj *prev;
    int     i;
    int     bit;

    i = 0;
    bit = 1;
    prev = NULL;
    do {
        if ((g_btl_actors[g_btl_actor_turn].targets & bit) != 0
            && g_btl_actors[i].c.key != 0
            && (signed char)g_btl_actors[i].c.status != BTL_STATUS_DOWN
            && (g_btl_actors[i].flags & BTL_ACTOR_OUT) == 0) {
            o = BtlOpenFxScatter(i, 0);
            o->mark_num++;
            if (prev != NULL) {
                BtlObjLast(o)->attached = prev;
            }
            prev = o;
        }
        i++;
        bit <<= 1;
    } while (i < BTL_ACTORS);
    o->mark_num = FX_MARK_HEAD;
    return o;
}

BtlObj *BtlOpenFxScatter(int slot, int timer)
{
    BtlObj *o;
    BtlObj *after;
    long    pos[3];
    int     i;

    i = FX_39_PIECES - 1;
    after = NULL;
    pos[0] = g_btl_actors[slot].obj->x;
    pos[1] = g_btl_actors[slot].obj->y;
    pos[2] = FX_39_DEPTH;
    do {
        g_btl_fx_def.scripts =
            ((const u_long ***)g_btl_unused_gfx)[i % FX_39_TABLES];
        o = BtlObjAlloc(&g_btl_fx_def, FX_OBJ_GROUP, 0, FX_OBJ_DRAW, 0, pos,
                        FX_OBJ_CD, FX_OBJ_CE);
        o->mark_num = i + FX_MARK_HEAD;
        o->attached = after;
        o->attr |= FX_39_ATTR;
        /* The table read is written first. The other way about, the cosine
           table's address is folded into the load; this way it is taken into
           a register ahead of the random fraction, the way the image has it. */
        o->shift_x = g_btl_wave_sin[i * FX_39_SPACING] * (rand() % FX_39_SHIFT);
        o->shift = g_btl_wave_cos[i * FX_39_SPACING] * (rand() % FX_39_SHIFT);
        after = o;
        o->timer = i * FX_39_STEP + timer;
        i--;
    } while (i >= 0);
    return o;
}
