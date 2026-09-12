/* Persona 1 (JP) - the move that rings the fighter twice over.  BTLP only.
 *   0x800BBD28 BtlFxStep44
 *
 * Seven moves share this one. The record waits for its timer, uncovers itself,
 * and on the next phase opens twelve more out of the artwork's second script
 * table - two turns round the six places BtlOpenFxRing uses, each one starting
 * four frames later than the last, and each threaded onto the one before. The
 * twelve carry the copy mark and do nothing here but be stepped.
 *
 * From then on the head record darkens and drifts a unit a frame toward the
 * camera, and the hit is armed the frame all three of its colours reach nought.
 * The copies free themselves at the same moment.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/object.h>
#include <persona/btlp/spellfx.h>

/* The mark a copy carries. */
#define FX_COPY_MARK 0xFF

/* Two turns round the six places, out of the second script table. */
#define FX_44_RING  6
#define FX_44_COUNT 12
#define FX_44_TABLE 1

/* What a copy carries, and how much later each one starts. */
#define FX_44_MOTION  2
#define FX_44_STAGGER 4

/* How fast the head darkens and how far it drifts each frame. */
#define FX_44_FADE 2
#define FX_44_NEAR 0x10000

/* What the last phase leaves behind. */
#define FX_44_DONE 0x80

/* 93.30%: the loop's tail. The image works the stagger, the counter's step and
   the record's kind out as soon as the allocation returns and stores them at
   the end; gcc here computes each at its own store, whether they are written
   inline, stepped by hand or held in temps. */
#ifdef NON_MATCHING
void BtlFxStep44(BtlObj *o)
{
    BtlObj *prev;
    long    pos[3];
    int     i;

    switch (o->phase) {
    case 0:
        if (o->timer != 0) {
            return;
        }
        o->attr &= ~(BTL_OBJ_HIDDEN | BTL_OBJ_STATIC);
        if (o->mark_num == FX_COPY_MARK) {
            return;
        }
        o->phase++;
        return;
    case 1:
        if ((o->attr & BTL_OBJ_ANIMATING) != 0) {
            return;
        }
        o->rgb_to[0] = 0;
        o->rgb_to[1] = 0;
        o->rgb_to[2] = 0;
        o->fade = FX_44_FADE;
        if (o->mark_num == 0) {
            g_btl_fx_def.scripts =
                ((const u_long ***)g_btl_unused_gfx)[FX_44_TABLE];
            i = 0;
            prev = 0;
            for (; i < FX_44_COUNT; ) {
                pos[0] = g_btl_actors[g_btl_fx_target].obj->x
                         + g_btl_fx_ring[i % FX_44_RING][0];
                pos[1] = g_btl_actors[g_btl_fx_target].obj->y
                         + g_btl_fx_ring[i % FX_44_RING][1];
                pos[2] = 0;
                prev = BtlObjAlloc(&g_btl_fx_def, FX_OBJ_GROUP, prev,
                                   FX_OBJ_DRAW, 0, pos, FX_OBJ_CD, FX_OBJ_CE);
                prev->attr = FX_OBJ_ATTR;
                prev->motion = FX_44_MOTION;
                prev->mark_num = FX_COPY_MARK;
                prev->timer = i * FX_44_STAGGER;
                i++;
                prev->kind = o->kind;
            }
        }
        o->phase++;
        return;
    case 2:
        o->z -= FX_44_NEAR;
        if ((o->rgb[0] | o->rgb[1] | o->rgb[2]) != 0) {
            return;
        }
        if (o->mark_num != 0) {
            BtlObjFree(o);
            return;
        }
        o->attr |= BTL_OBJ_HIDDEN;
        BtlArmHitOne();
        o->phase = FX_44_DONE;
        o->children = 0;
        return;
    default:
        g_btl_spell_fx[g_btl_fx_move].finish(o);
        return;
    }
}
#else
INCLUDE_ASM("btlp/nonmatchings/fxstep44", BtlFxStep44);
#endif
