/* Persona 1 (JP) - moves 0x12 and 0x2D's effect: pairs of records over the far
 * side, and a ring of eight above it.  BTLP only.
 *   0x800B92AC BtlFxStart12
 *
 * A start handler out of g_btl_spell_fx. On every other row and column of the
 * far side's grid it opens two records, one out of the staged artwork's third
 * script table and one out of its fourth, each marked as a copy and arriving
 * at a random frame of the first FX_12_SCATTER. Then, a hundred units past the
 * middle toward that side, a ring of eight out of the second table, spaced
 * evenly round the wave tables, each marked by its place in the ring counted
 * up from FX_MARK_HEAD and gliding for FX_12_STEPS frames. The last of the
 * ring is the one answered.
 */
#include <decomp/types.h>
#include <rand.h>
#include <persona/btlp/formation.h>
#include <persona/btlp/object.h>
#include <persona/btlp/round.h>
#include <persona/btlp/spellfx.h>

/* The pairs: three rows and three columns of the grid's five, how many
   layers each cell has and how far apart they rise, the two tables, and the
   frames an arrival is scattered over. */
#define FX_12_ROWS       3
#define FX_12_COLS       3
#define FX_12_LAYERS     1
#define FX_12_RISE       (-32)
#define FX_12_LOW_TABLE  2
#define FX_12_HIGH_TABLE 3
#define FX_12_SCATTER    30

/* The ring: its table, how many records, how far past the middle it stands,
   how far round the wave tables each is from the next, and its glide. */
#define FX_12_RING_TABLE 1
#define FX_12_RING       8
#define FX_12_RING_Y     0x640000
#define FX_12_RING_SPACE 0x40
#define FX_12_STEPS      0x1E

BtlObj *BtlFxStart12(void)
{
    BtlObj *o;
    BtlObj *after;
    long    pos[3];
    int     row;
    int     col;
    int     layer;
    int     i;

    after = NULL;
    row = FX_12_ROWS - 1;
    do {
        col = 0;
        do {
            layer = 0;
            do {
                /* The depth is written last in both arms, and as the product
                   it is: behind the test its store is scheduled among the
                   call's arguments, and as a running value it costs a
                   register the image does not spend. */
                if (g_btl_actor_turn < BTL_PARTY) {
                    pos[0] = (col * 2 * PLACE_COL_W + PLACE_COL_ORG)
                             * PLACE_FIXED;
                    pos[1] = (row * 2 * PLACE_ROW_H + PLACE_COL_ORG
                              - (FX_GRID_H - 1) * PLACE_ROW_H) * PLACE_FIXED;
                    pos[2] = layer * FX_12_RISE * PLACE_FIXED;
                } else {
                    pos[0] = (col * 2 * PLACE_COL_W + PLACE_COL_ORG)
                             * PLACE_FIXED;
                    pos[1] = (row * 2 * PLACE_ROW_H + PLACE_ROW_ORG)
                             * PLACE_FIXED;
                    pos[2] = layer * FX_12_RISE * PLACE_FIXED;
                }
                g_btl_fx_def.scripts =
                    ((const u_long ***)g_btl_unused_gfx)[FX_12_LOW_TABLE];
                o = BtlObjAlloc(&g_btl_fx_def, FX_OBJ_GROUP, after, FX_OBJ_DRAW,
                                0, pos, FX_OBJ_CD, FX_OBJ_CE);
                o->attr = FX_OBJ_ATTR;
                o->attached = after;
                o->mark_num = FX_COPY_MARK;
                o->timer = rand() % FX_12_SCATTER;
                after = o;
                g_btl_fx_def.scripts =
                    ((const u_long ***)g_btl_unused_gfx)[FX_12_HIGH_TABLE];
                o = BtlObjAlloc(&g_btl_fx_def, FX_OBJ_GROUP, after, FX_OBJ_DRAW,
                                0, pos, FX_OBJ_CD, FX_OBJ_CE);
                o->attr = FX_OBJ_ATTR;
                o->attached = after;
                o->mark_num = FX_COPY_MARK;
                o->timer = rand() % FX_12_SCATTER;
                after = o;
                layer++;
            } while (layer < FX_12_LAYERS);
            col++;
        } while (col < FX_12_COLS);
        row--;
    } while (row >= 0);

    g_btl_fx_def.scripts = ((const u_long ***)g_btl_unused_gfx)[FX_12_RING_TABLE];
    i = FX_12_RING - 1;
    do {
        if (g_btl_actor_turn < BTL_PARTY) {
            pos[0] = 0;
            pos[1] = -FX_12_RING_Y;
            pos[2] = 0;
        } else {
            pos[0] = 0;
            pos[1] = FX_12_RING_Y;
            pos[2] = 0;
        }
        o = BtlObjAlloc(&g_btl_fx_def, FX_OBJ_GROUP, after, FX_OBJ_DRAW, 0, pos,
                        FX_OBJ_CD, FX_OBJ_CE);
        o->attr = FX_OBJ_ATTR;
        o->attached = after;
        after = o;
        o->mark_num = i + FX_MARK_HEAD;
        o->steps = FX_12_STEPS;
        o->angle = i * FX_12_RING_SPACE;
        i--;
    } while (i >= 0);
    return o;
}
