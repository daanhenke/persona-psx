/* Persona 1 (JP) - the arc that lands on the far side, and two moves that
 * throw one.  BTLP only.
 *   0x800B79E0 BtlFxStepUnused  0x800B7AC4 BtlFxStep09
 *   0x800B7CFC BtlFxStart0D     0x800B7D74 BtlFxStart0E
 *
 * BtlFxStep09 carries its record along a falling arc - a standing step across
 * the field and a step in depth, both of which grow every frame - and when it
 * reaches the floor it leaves a copy behind where it landed. The record that
 * carries the head mark arms the hit there; anything else frees itself. The
 * copies do nothing at all.
 *
 * BtlFxStart0D and BtlFxStart0E open one ordinary record on the fighter aimed
 * at and set it going up and back, the second twice as far as the first and
 * for twice as long. Neither is this file's own step handler: both moves are
 * carried by the plain one.
 *
 * BtlFxStepUnused in front of them is a step handler nothing reaches - no
 * table entry, no call, no word in BTLP.BIN holding its address - the same way
 * BtlFxStepUnused2 and move 0x0F's second start handler are.
 */
#include <decomp/types.h>
#include <persona/btlp/object.h>
#include <persona/btlp/round.h>
#include <persona/btlp/spellfx.h>

/* The mark a landed copy carries, and the one the head of a pair carries. */
#define FX_COPY_MARK 0xFF
#define FX_09_HEAD   0x20

/* How fast the arc starts across the field and how fast it falls, both of
   which are added to again every frame. */
#define FX_09_ACROSS 0x10000
#define FX_09_FALL   0x8000

/* What the two throws are given: how far back they start and how long they
   are allowed. */
#define FX_0D_RISE 0x20000
#define FX_0D_BACK 0x140000
#define FX_0D_LIFE 0xA
#define FX_0E_BACK 0x280000
#define FX_0E_LIFE 0x14

/* What a step handler leaves behind, and what the hit counts. */
#define FX_STEP_DONE 0x80
#define FX_STEP_HITS 2

void BtlFxStepUnused(BtlObj *o)
{
    switch (o->phase) {
    case 0:
        if (o->timer != 0) {
            return;
        }
        o->attr = (o->attr & ~(BTL_OBJ_HIDDEN | BTL_OBJ_STATIC))
                  | BTL_OBJ_ANIMATING;
        o->phase++;
        return;
    case 1:
        if (o->mark_num == 0) {
            if ((o->attr & BTL_OBJ_ANIMATING) != 0) {
                break;
            }
            o->attr |= BTL_OBJ_HIDDEN;
            BtlArmHitOne();
            o->phase = FX_STEP_DONE;
            o->children = FX_STEP_HITS;
        } else {
            if ((o->attr & BTL_OBJ_ANIMATING) != 0) {
                break;
            }
            BtlObjFree(o);
        }
        break;
    default:
        break;
    }
    BtlFinishMoveFx(o);
}

void BtlFxStep09(BtlObj *o)
{
    BtlObj *n;
    const u_long ***scripts;

    if (o->mark_num == FX_COPY_MARK) {
        return;
    }
    switch (o->phase) {
    case 0:
        if (o->timer != 0) {
            return;
        }
        o->attr &= ~(BTL_OBJ_HIDDEN | BTL_OBJ_STATIC);
        if (g_btl_actor_turn < BTL_PARTY) {
            o->step_y = -FX_09_ACROSS;
        } else {
            o->step_y = FX_09_ACROSS;
        }
        o->step_z = FX_09_FALL;
        o->phase++;
        return;
    case 1:
        scripts = &g_btl_fx_def.scripts;
        *scripts = ((const u_long ***)g_btl_unused_gfx)[1];
        if (o->mark_num == FX_09_HEAD) {
            if (o->z >= 0) {
                o->z = 0;
                n = BtlObjAlloc((BtlObjDef *)(scripts - 1), FX_OBJ_GROUP, 0,
                                FX_OBJ_DRAW, 0, &o->x, FX_OBJ_CD, FX_OBJ_CE);
                n->kind = o->kind;
                n->mark_num = FX_COPY_MARK;
                o->attr |= BTL_OBJ_HIDDEN;
                BtlArmHitChain();
                o->phase = FX_STEP_DONE;
                o->children = FX_STEP_HITS;
            } else {
                o->y += o->step_y;
                o->z += o->step_z;
            }
        } else {
            if (o->z >= 0) {
                o->z = 0;
                n = BtlObjAlloc((BtlObjDef *)(scripts - 1), FX_OBJ_GROUP, 0,
                                FX_OBJ_DRAW, 0, &o->x, FX_OBJ_CD, FX_OBJ_CE);
                n->kind = o->kind;
                n->mark_num = FX_COPY_MARK;
                BtlObjFree(o);
            } else {
                o->y += o->step_y;
                o->z += o->step_z;
            }
        }
        o->step_y += (g_btl_actor_turn < BTL_PARTY) ? -FX_09_ACROSS
                                                    : FX_09_ACROSS;
        o->step_z += FX_09_FALL;
        return;
    default:
        g_btl_spell_fx[g_btl_fx_move].finish(o);
        return;
    }
}

BtlObj *BtlFxStart0D(void)
{
    BtlObj *o;

    o = BtlOpenFxObj2(g_btl_fx_target, FX_TIMER);
    if (g_btl_actor_turn < BTL_PARTY) {
        o->step_y = -FX_0D_RISE;
        o->y += FX_0D_BACK;
    } else {
        o->step_y = FX_0D_RISE;
        o->y -= FX_0D_BACK;
    }
    o->steps = FX_0D_LIFE;
    return o;
}

BtlObj *BtlFxStart0E(void)
{
    BtlObj *o;

    o = BtlOpenFxObj2(g_btl_fx_target, FX_TIMER);
    if (g_btl_actor_turn < BTL_PARTY) {
        o->step_y = -FX_0D_RISE;
        o->y += FX_0E_BACK;
    } else {
        o->step_y = FX_0D_RISE;
        o->y -= FX_0E_BACK;
    }
    o->steps = FX_0E_LIFE;
    return o;
}
