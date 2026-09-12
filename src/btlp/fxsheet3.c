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

    row     = FX_GRID_H - 1;
    cell    = FX_GRID_W * FX_GRID_H - 1;
    after   = 0;
    y_party = FX_GRID_Y_PARTY;
    y_enemy = FX_GRID_Y_ENEMY;

    BtlTintTargets(g_btl_actor_turn, g_btl_tint_fx_r, g_btl_tint_fx_g,
                   g_btl_tint_fx_b);

    g_btl_fx_def.scripts = ((const u_long ***)g_btl_unused_gfx)[0];

    for (; row >= 0; row--)
    {
        col = 0;
        x   = FX_GRID_X0;
        for (; col < FX_GRID_W; col++)
        {
            if (g_btl_actor_turn < BTL_PARTY)
            {
                pos[0] = x;
                pos[1] = y_enemy;
            }
            else
            {
                pos[0] = x;
                pos[1] = y_party;
            }
            pos[2] = 0;

            o = BtlObjAlloc(&g_btl_fx_def, FX_OBJ_GROUP, after, FX_OBJ_DRAW, 0,
                            pos, FX_OBJ_CD, FX_OBJ_CE);
            o->attr     = FX_OBJ_ATTR;
            o->attached = after;
            after = o;
            x    += FX_GRID_DX;
            o->mark_num = cell + FX_MARK_FAR;
            o->timer    = (g_btl_fx_grid_order[cell + FX_MARK_FAR] >> 1)
                        + FX_GRID_LATE;
            cell--;
        }
        /* Both rows step by the same amount, so the compiler shares the one
           constant between them and then lifts it clear of the loop; the
           image works it out afresh on every row. */
        y_party -= FX_GRID_DY;
        y_enemy -= FX_GRID_DY;
    }
    return o;
}
#else
INCLUDE_ASM("btlp/nonmatchings/fxsheet3", BtlFxStartSheetLate);
#endif
