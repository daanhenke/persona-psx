/* Persona 1 (JP) - the move whose sheet arrives eight frames late.
 * BTLP only.
 *   0x800B6CCC BtlFxStartSheetLate
 *
 * The same five-by-five sheet BtlOpenFxGrid lays over a side, written out
 * again here rather than called: this one has no own-side case and no lift off
 * the floor, and every cell waits eight frames longer than the scatter table
 * asks for.
 *
 * Everything the move reaches is put on the effect's own colour first.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <persona/btlp/formation.h>
#include <persona/btlp/object.h>
#include <persona/btlp/round.h>
#include <persona/btlp/spellfx.h>

/* How long every cell waits on top of its own entry in the scatter. */
#define FX_GRID_LATE 8

/* 95.74%. The link to the previous cell is written in both arms of an
   identical test; cross-jumping folds them, but at loop time they are what
   push the outer loop over loop.c's budget, so the row step stays in the loop
   as the image has it. Left: the image loads the cell's attribute word into
   v1 straight after the allocation call, ahead of the copy of its result;
   here it is built in v0 at the store. That is the shape of a constant
   loop.c lifted and reload rebuilt in a spare register, which sched2 then
   hoists, but the inner loop is too large for loop.c to lift it here. */
#ifdef NON_MATCHING
BtlObj *BtlFxStartSheetLate(void)
{
    BtlObj *o;
    BtlObj *after;
    long    pos[3];
    long    x;
    long    y_party;
    long    y_enemy;
    int     row;
    int     col;
    int     cell;
    int     k;

    row   = FX_GRID_H - 1;
    cell  = FX_GRID_W * FX_GRID_H - 1;
    after = 0;
    BtlTintTargets(g_btl_actor_turn, g_btl_tint_fx_r, g_btl_tint_fx_g,
                   g_btl_tint_fx_b);
    g_btl_fx_def.scripts = ((const u_long ***)g_btl_unused_gfx)[0];
    y_party = FX_GRID_Y_PARTY;
    y_enemy = FX_GRID_Y_ENEMY;

    do {
        col = 0;
        k   = cell + FX_MARK_FAR;
        x   = FX_GRID_X0;
        do {
            if (g_btl_actor_turn < BTL_PARTY) {
                pos[0] = x;
                pos[1] = y_enemy;
                pos[2] = 0;
            } else {
                pos[0] = x;
                pos[1] = y_party;
                pos[2] = 0;
            }
            o = BtlObjAlloc(&g_btl_fx_def, FX_OBJ_GROUP, after, FX_OBJ_DRAW, 0,
                            pos, FX_OBJ_CD, FX_OBJ_CE);
            if (g_btl_actor_turn < BTL_PARTY) {
                o->attached = after;
            } else {
                o->attached = after;
            }
            after = o;
            x += FX_GRID_DX;
            col++;
            o->attr     = FX_OBJ_ATTR;
            o->mark_num = k;
            o->timer    = (g_btl_fx_grid_order[k] >> 1) + FX_GRID_LATE;
            k--;
            cell--;
        } while (col < FX_GRID_W);
        y_party -= FX_GRID_DY;
        y_enemy -= FX_GRID_DY;
        row--;
    } while (row >= 0);
    return o;
}
#else
INCLUDE_ASM("btlp/nonmatchings/fxsheet3", BtlFxStartSheetLate);
#endif
