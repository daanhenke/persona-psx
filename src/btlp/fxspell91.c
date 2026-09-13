/* Persona 1 (JP) - the move drawn as nine pieces round the fighter.
 * BTLP only.
 *   0x800BED04 BtlFxStart91
 *
 * A start handler out of g_btl_spell_fx, and the one the rest of the moves in
 * fxspell92.c borrow. The same build as BtlFxStart90 from the nine entries of
 * g_btl_fx_spread_91: the box again, with the sides and the ends between its
 * corners. Made from the last entry back, so the first one
 * made arrives at once, the next four are held six frames and the last four -
 * the head of the chain among them - twelve.
 */
#include <decomp/types.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/object.h>
#include <persona/btlp/spellfx.h>

/* How many pieces there are, and the two holds - the shorter for the pieces
   from FX_91_NEAR up. */
#define FX_91_PIECES 9
#define FX_91_NEAR   4
#define FX_91_HOLD   0xC
#define FX_91_SHORT  6

BtlObj *BtlFxStart91(void)
{
    BtlObj *o;
    BtlObj *after;
    BtlObj *on;
    long    pos[3];
    int     i;

    g_btl_fx_def.scripts = ((const u_long ***)g_btl_unused_gfx)[0];
    i = FX_91_PIECES - 1;
    after = 0;
    on = g_btl_actors[g_btl_fx_target].obj;
    do {
        pos[0] = on->x + g_btl_fx_spread_91[i][0];
        pos[1] = on->y + g_btl_fx_spread_91[i][1];
        pos[2] = 0;
        o = BtlObjAlloc(&g_btl_fx_def, FX_OBJ_GROUP, 0, FX_OBJ_DRAW, 0, pos,
                        FX_OBJ_CD, FX_OBJ_CE);
        o->attr |= FX_OBJ_ATTR;
        if (i == FX_91_PIECES - 1) {
            o->timer = 0;
        } else {
            o->timer = i < FX_91_NEAR ? FX_91_HOLD : FX_91_SHORT;
        }
        o->attached = after;
        after = o;
        o->mark_num = i + FX_MARK_HEAD;
        i--;
    } while (i >= 0);
    return o;
}
