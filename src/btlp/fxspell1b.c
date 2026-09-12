/* Persona 1 (JP) - a chain of move 0x19's effect, and a sheet drawn twice
 * over.  BTLP only.
 *   0x800B84CC BtlFxStart1B  0x800B8560 BtlFxStart1C
 *
 * Two start handlers out of g_btl_spell_fx. The first builds move 0x19's pair
 * five times over and threads the five onto one another by the end of each
 * chain rather than by its head, which is what keeps all ten records on one
 * list; the head's own mark is put back to nought at the end so the step
 * handler finds it again.
 *
 * The second lays the same five-by-five sheet BtlOpenFxGrid does over the side
 * that is being aimed at, but opens two records per cell: the one the scatter
 * table times, and a copy of it carrying the cell's own mark and the same
 * timer. Everything the move reaches is put on white first.
 */
#include <decomp/types.h>
#include <persona/btlp/formation.h>
#include <persona/btlp/object.h>
#include <persona/btlp/round.h>
#include <persona/btlp/spellfx.h>

/* How many times move 0x19's pair is built, what each one is marked with
   while the chain is being made, and how much later each starts. */
#define FX_1B_COUNT   5
#define FX_1B_MARK    1
#define FX_1B_STAGGER 4

/* The two attribute bits every record of the chain is given. */
#define FX_1B_ATTR 0xC0000

/* What the timed record of each pair starts as - the plain effect attribute
   and the two bits move 0x19's back record also carries. */
#define FX_1C_ATTR (FX_OBJ_ATTR | BTL_OBJ_PICKED | BTL_OBJ_ATTR_4000)

BtlObj *BtlFxStart1B(void)
{
    BtlObj *o;
    BtlObj *after;
    int     i;
    int     mark;

    i = 0;
    after = 0;
    mark = FX_1B_MARK;
    for (; i < FX_1B_COUNT; i++) {
        o = BtlFxStart19();
        o->mark_num = mark;
        BtlObjSetAttr(o, FX_1B_ATTR);
        BtlObjSetTimer(o, i * FX_1B_STAGGER);
        BtlObjLast(o)->attached = after;
        after = o;
    }
    o->mark_num = 0;
    return o;
}

BtlObj *BtlFxStart1C(void)
{
    BtlObj *o;
    BtlObj *twin;
    long    pos[3];
    long    x;
    long    y_party;
    long    y_enemy;
    int     row;
    int     col;
    int     cell;
    int     mark;

    BtlTintTargets(g_btl_actor_turn, FX_WHITE, FX_WHITE, FX_WHITE);

    row  = FX_GRID_H - 1;
    cell = FX_GRID_W * FX_GRID_H - 1;
    o    = 0;

    g_btl_fx_def.scripts = ((const u_long ***)g_btl_unused_gfx)[0];
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
                pos[2] = 0;
            }
            else
            {
                pos[0] = x;
                pos[1] = y_party;
                pos[2] = 0;
            }

            twin = BtlObjAlloc(&g_btl_fx_def, FX_OBJ_GROUP, o, FX_OBJ_DRAW, 0,
                               pos, FX_OBJ_CD, FX_OBJ_CE);
            twin->attr     = FX_1C_ATTR;
            twin->attached = o;
            o = twin;
            col++;
            o->mark_num = FX_MARK_REST;
            o->timer    = g_btl_fx_grid_order[mark] >> 1;

            twin = BtlObjClone(o);
            twin->attr     = FX_OBJ_ATTR;
            twin->attached = o;
            twin->mark_num = mark;
            twin->timer    = o->timer;
            o = twin;

            x += FX_GRID_DX;
            mark--;
            cell--;
        }
        y_party -= FX_GRID_DY;
        y_enemy -= FX_GRID_DY;
    }
    return twin;
}
