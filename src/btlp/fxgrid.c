/* Persona 1 (JP) - an effect spread over a whole side of the field.
 * BTLP only.
 *   0x800C03B0 BtlOpenFxGrid
 *
 * Twenty-five effect records, one on every cell of a side's formation grid,
 * all chained together and answered as one. The cells are the same thirty by
 * twenty pixels the fighters stand on, so the sheet lies exactly over the side
 * it covers.
 *
 * Which side that is falls out of who is acting: a move goes to the far side,
 * and the one move that does not goes to the acting fighter's own and stands a
 * hundred units above the floor rather than on it.
 *
 * The cells do not arrive in order. Each is given the timer its entry in
 * g_btl_fx_grid_order carries, which scatters the sheet: reading a cell's
 * entry out of the table by the same number the record is marked with is what
 * makes the two agree.
 *
 * The last record made is the one answered, and the chain runs back from it to
 * the first.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/formation.h>
#include <persona/btlp/object.h>
#include <persona/btlp/round.h>
#include <persona/btlp/spellfx.h>

/* How far off the floor the own-side sheet stands. */
#define FX_GRID_Z_LIFT (-0x64 * PLACE_FIXED)

/* The move whose sheet covers the acting fighter's own side instead of the
   far one. */
#define FX_MOVE_OWN_SIDE 9

/* The own-side sheet's cells are biased further so the two are told
   apart. */
#define FX_MARK_OWN 0x20

#ifdef NON_MATCHING
BtlObj *BtlOpenFxGrid(int table)
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
    y_enemy = FX_GRID_Y_ENEMY;

    g_btl_fx_def.scripts = ((const u_long ***)g_btl_unused_gfx)[table];

    y_party = FX_GRID_Y_PARTY;

    for (; row >= 0; row--)
    {
        x = FX_GRID_X0;
        for (col = 0; col < FX_GRID_W; col++)
        {
            if (g_btl_fx_move == FX_MOVE_OWN_SIDE)
            {
                if (g_btl_actor_turn < BTL_PARTY)
                {
                    pos[0] = x;
                    pos[1] = y_party;
                    pos[2] = FX_GRID_Z_LIFT;
                }
                else
                {
                    pos[0] = x;
                    pos[1] = y_enemy;
                    pos[2] = FX_GRID_Z_LIFT;
                }
            }
            else
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
            }

            o = BtlObjAlloc(&g_btl_fx_def, FX_OBJ_GROUP, after, FX_OBJ_DRAW, 0,
                            pos, FX_OBJ_CD, FX_OBJ_CE);
            o->attr |= FX_OBJ_ATTR;
            o->attached = after;
            after = o;
            x += FX_GRID_DX;

            o->mark_num = g_btl_fx_move == FX_MOVE_OWN_SIDE
                              ? cell + FX_MARK_OWN
                              : cell + FX_MARK_FAR;
            o->timer = g_btl_fx_grid_order[cell + FX_MARK_FAR] >> 1;
            cell--;
        }
        y_enemy -= FX_GRID_DY;
        y_party -= FX_GRID_DY;
    }
    return o;
}
#else
INCLUDE_ASM("btlp/nonmatchings/fxgrid", BtlOpenFxGrid);
#endif
