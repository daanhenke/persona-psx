/* Persona 1 (JP) - the move that scatters twenty-five records over a side and
 * shakes the field as it lands.  BTLP only.
 *   0x800B81D0 BtlFxStart15
 *
 * A start handler out of g_btl_spell_fx. It lays the same five-by-five sheet
 * BtlOpenFxGrid does, but each cell is drawn from one of the staged artwork's
 * five script tables picked at random, so no two playings of the move look
 * alike. The template's script pointer is rewritten in place before every
 * allocation, which is why the address of that one field is worth a local of
 * its own - and why the template's own address is worked out from it rather
 * than named a second time, which is all the -1 on the call is.
 *
 * The shake flag is raised once for the frame the sheet opens on.
 */
#include <decomp/types.h>
#include <rand.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/object.h>
#include <persona/btlp/round.h>
#include <persona/btlp/spellfx.h>

/* How many of the staged artwork's script tables a cell may be drawn from. */
#define FX_15_TABLES 5

BtlObj *BtlFxStart15(void)
{
    BtlObj *o;
    BtlObj *prev;
    const u_long ***scripts;
    long    pos[3];
    long    x;
    long    y_party;
    long    y_enemy;
    int     row;
    int     col;
    int     cell;
    int     mark;

    g_btl_shake_on = 1;

    row     = FX_GRID_H - 1;
    cell    = FX_GRID_W * FX_GRID_H - 1;
    prev    = 0;
    scripts = &g_btl_fx_def.scripts;
    y_party = FX_GRID_Y_PARTY;
    y_enemy = FX_GRID_Y_ENEMY;

    for (; row >= 0; row--)
    {
        col  = 0;
        mark = cell + FX_MARK_FAR;
        x    = FX_GRID_X0;
        for (; col < FX_GRID_W; )
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

            *scripts =
                ((const u_long ***)g_btl_unused_gfx)[rand() % FX_15_TABLES];
            col++;
            cell--;
            o = BtlObjAlloc((BtlObjDef *)(scripts - 1), FX_OBJ_GROUP, prev,
                            FX_OBJ_DRAW, 0,
                            pos, FX_OBJ_CD, FX_OBJ_CE);
            o->attr     = FX_OBJ_ATTR;
            o->attached = prev;
            prev = o;
            x += FX_GRID_DX;
            o->mark_num = mark;
            o->timer    = g_btl_fx_grid_order[mark] >> 1;
            mark--;
        }
        y_party -= FX_GRID_DY;
        y_enemy -= FX_GRID_DY;
    }
    return o;
}
