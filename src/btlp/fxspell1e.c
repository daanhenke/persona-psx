/* Persona 1 (JP) - move 0x1E's effect: a nine-cell sheet, and a stack of
 * seventy-five records over the whole far side.  BTLP only.
 *   0x800B8718 BtlFxStart1E
 *
 * A start handler out of g_btl_spell_fx. Everything the move reaches is put on
 * white first. Then nine records are laid over the far side, on every other
 * row and column of its grid, out of the staged artwork's first script table;
 * each arrives when g_btl_fx_nine_order says, and twice as late when the
 * acting fighter's move is FX_1E_SLOW. Over them, out of the second table,
 * every cell of the side gets a column of three records rising thirty-two
 * units apart, each arriving at a random frame of the first FX_1E_SCATTER. The
 * seventy-five are marked by their place in the build counted down to
 * FX_MARK_HEAD, and the last one made is answered.
 */
#include <decomp/types.h>
#include <rand.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/formation.h>
#include <persona/btlp/object.h>
#include <persona/btlp/round.h>
#include <persona/btlp/spellfx.h>

/* What the targets are put on. */
#define FX_1E_WHITE 0xFF

/* The nine: three rows and three columns of the grid's five, their table, and
   the move that has them arrive twice as late. */
#define FX_1E_NINE_ROWS  3
#define FX_1E_NINE_COLS  3
#define FX_1E_NINE_TABLE 0
#define FX_1E_SLOW       0x30

/* The stack: its table, how many layers each cell has and how far apart they
   rise, the frames an arrival is scattered over, and what every record of it
   carries on top of the plain effect attribute. */
#define FX_1E_STACK_TABLE 1
#define FX_1E_LAYERS      3
#define FX_1E_RISE        (-32)
#define FX_1E_SCATTER     30
#define FX_1E_ATTR        (FX_OBJ_ATTR | BTL_OBJ_PICKED)

BtlObj *BtlFxStart1E(void)
{
    BtlObj *o;
    BtlObj *after;
    long    pos[3];
    int     row;
    int     col;
    int     layer;
    int     cell;

    BtlTintTargets(g_btl_actor_turn, FX_1E_WHITE, FX_1E_WHITE, FX_1E_WHITE);
    cell = 0;
    after = NULL;
    row = 0;
    g_btl_fx_def.scripts = ((const u_long ***)g_btl_unused_gfx)[FX_1E_NINE_TABLE];
    do {
        col = 0;
        do {
            if (g_btl_actor_turn < BTL_PARTY) {
                pos[0] = (col * 2 * PLACE_COL_W + PLACE_COL_ORG) * PLACE_FIXED;
                pos[1] = (row * 2 * PLACE_ROW_H + PLACE_COL_ORG
                          - (FX_GRID_H - 1) * PLACE_ROW_H) * PLACE_FIXED;
                pos[2] = 0;
            } else {
                pos[0] = (col * 2 * PLACE_COL_W + PLACE_COL_ORG) * PLACE_FIXED;
                pos[1] = (row * 2 * PLACE_ROW_H + PLACE_ROW_ORG) * PLACE_FIXED;
                pos[2] = 0;
            }
            o = BtlObjAlloc(&g_btl_fx_def, FX_OBJ_GROUP, after, FX_OBJ_DRAW, 0,
                            pos, FX_OBJ_CD, FX_OBJ_CE);
            o->mark_num = FX_MARK_REST;
            o->attr = FX_OBJ_ATTR;
            o->attached = after;
            o->timer = g_btl_actors[g_btl_actor_turn].move == FX_1E_SLOW
                           ? g_btl_fx_nine_order[cell] << 1
                           : g_btl_fx_nine_order[cell];
            after = o;
            col++;
            cell++;
        } while (col < FX_1E_NINE_COLS);
        row++;
    } while (row < FX_1E_NINE_ROWS);

    g_btl_fx_def.scripts = ((const u_long ***)g_btl_unused_gfx)[FX_1E_STACK_TABLE];
    row = FX_GRID_H - 1;
    /* The nine's counter counts the stack's marks down. A local of its own
       takes the saved register the column's x is copied into. */
    cell = FX_GRID_W * FX_GRID_H * FX_1E_LAYERS - 1;
    do {
        col = 0;
        do {
            layer = 0;
            do {
                /* Depth last in both arms, as a product - see fxspell12.c. */
                if (g_btl_actor_turn < BTL_PARTY) {
                    pos[0] = (col * PLACE_COL_W + PLACE_COL_ORG) * PLACE_FIXED;
                    pos[1] = (row * PLACE_ROW_H + PLACE_COL_ORG
                              - (FX_GRID_H - 1) * PLACE_ROW_H) * PLACE_FIXED;
                    pos[2] = layer * FX_1E_RISE * PLACE_FIXED;
                } else {
                    pos[0] = (col * PLACE_COL_W + PLACE_COL_ORG) * PLACE_FIXED;
                    pos[1] = (row * PLACE_ROW_H + PLACE_ROW_ORG) * PLACE_FIXED;
                    pos[2] = layer * FX_1E_RISE * PLACE_FIXED;
                }
                o = BtlObjAlloc(&g_btl_fx_def, FX_OBJ_GROUP, after, FX_OBJ_DRAW,
                                0, pos, FX_OBJ_CD, FX_OBJ_CE);
                o->attr = FX_1E_ATTR;
                o->attached = after;
                o->mark_num = cell + FX_MARK_HEAD;
                o->timer = rand() % FX_1E_SCATTER;
                after = o;
                layer++;
                cell--;
            } while (layer < FX_1E_LAYERS);
            col++;
        } while (col < FX_GRID_W);
        row--;
    } while (row >= 0);
    return o;
}
