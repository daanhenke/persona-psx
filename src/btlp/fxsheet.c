/* Persona 1 (JP) - the moves whose effect is a sheet over a whole side.
 * BTLP only.
 *   0x800B6F70 BtlFxStart05  0x800B71B0 BtlFxStartSheet
 *
 * Start handlers out of g_btl_spell_fx. Both put everything the move reaches
 * on the effect's own colour first, so the fighters change colour under the
 * sheet rather than behind it.
 *
 * BtlFxStart05's sheet is a coarse one in three layers: on every other row of
 * the far side's grid - its front, its middle and its back - and in every
 * column, three records are opened on the one point, the middle one out of the
 * staged artwork's first script table and the two beside it out of the second,
 * each pushed across by its layer's entry in g_btl_fx_shift. The middle layer
 * arrives first and the other two eight and sixteen frames after it, and each
 * row four frames after the one in front of it, so the sheet spreads away from
 * the middle of the field. Every record is marked by its cell and its layer
 * together, and the one that comes out at FX_MARK_HEAD - the middle layer of
 * the last cell - is the one answered.
 *
 * BtlFxStartSheet opens BtlOpenFxGrid's five-by-five sheet over the side.
 */
#include <decomp/types.h>
#include <persona/btlp/formation.h>
#include <persona/btlp/object.h>
#include <persona/btlp/round.h>
#include <persona/btlp/spellfx.h>

/* Which of the staged script tables the sheet's cells are drawn from. */
#define FX_SHEET_TABLE 0

/* BtlFxStart05's grid: three of the side's five rows, every column, three
   layers on each cell; the tables the middle layer and the two beside it come
   out of; how much later each layer and each row arrives than the one before
   it; and what every record is given on top of the plain effect attribute. */
#define FX_05_ROWS       3
#define FX_05_LAYERS     3
#define FX_05_MID_TABLE  0
#define FX_05_SIDE_TABLE 1
#define FX_05_LAYER_WAIT 8
#define FX_05_ROW_WAIT   4
#define FX_05_ATTR       (FX_OBJ_ATTR | BTL_OBJ_SHIFT_SCREEN | 0x80000)

BtlObj *BtlFxStart05(void)
{
    BtlObj *o;
    BtlObj *after;
    BtlObj *head;
    const u_long ***scripts;
    long    pos[3];
    int     row;
    int     col;
    int     layer;
    int     cell;

    row = FX_05_ROWS - 1;
    cell = FX_05_ROWS * FX_GRID_W - 1;
    after = NULL;
    scripts = &g_btl_fx_def.scripts;
    BtlTintTargets(g_btl_actor_turn, g_btl_tint_fx_r, g_btl_tint_fx_g,
                   g_btl_tint_fx_b);
    do {
        col = 0;
        do {
            layer = FX_05_LAYERS - 1;
            do {
                if (g_btl_actor_turn < BTL_PARTY) {
                    pos[0] = (col * PLACE_COL_W + PLACE_COL_ORG) * PLACE_FIXED;
                    pos[1] = (row * 2 * PLACE_ROW_H + PLACE_COL_ORG
                              - (FX_GRID_H - 1) * PLACE_ROW_H) * PLACE_FIXED;
                } else {
                    pos[0] = (col * PLACE_COL_W + PLACE_COL_ORG) * PLACE_FIXED;
                    pos[1] = (row * 2 * PLACE_ROW_H + PLACE_ROW_ORG)
                             * PLACE_FIXED;
                }
                pos[2] = 0;
                if (layer == 0) {
                    *scripts =
                        ((const u_long ***)g_btl_unused_gfx)[FX_05_MID_TABLE];
                } else {
                    *scripts =
                        ((const u_long ***)g_btl_unused_gfx)[FX_05_SIDE_TABLE];
                }
                o = BtlObjAlloc(&g_btl_fx_def, FX_OBJ_GROUP, after, FX_OBJ_DRAW,
                                0, pos, FX_OBJ_CD, FX_OBJ_CE);
                o->mark_num = cell + layer + FX_MARK_HEAD;
                o->attr = FX_05_ATTR;
                o->attached = after;
                o->timer = g_btl_actor_turn < BTL_PARTY
                    ? layer * FX_05_LAYER_WAIT
                          + ((FX_05_ROWS - 1) - row) * FX_05_ROW_WAIT
                    : layer * FX_05_LAYER_WAIT + row * FX_05_ROW_WAIT;
                o->shift_x = g_btl_fx_shift[layer] << 16;
                /* The chain is advanced ahead of the head test: behind it,
                   `after` and the column's x trade saved registers. */
                after = o;
                if (o->mark_num == FX_MARK_HEAD) {
                    head = o;
                }
                layer--;
            } while (layer >= 0);
            col++;
            cell--;
        } while (col < FX_GRID_W);
        row--;
    } while (row >= 0);
    return head;
}

BtlObj *BtlFxStartSheet(void)
{
    BtlTintTargets(g_btl_actor_turn, g_btl_tint_fx_r, g_btl_tint_fx_g,
                   g_btl_tint_fx_b);
    return BtlOpenFxGrid(FX_SHEET_TABLE);
}
