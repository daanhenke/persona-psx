/* Persona 1 (JP) - the move drawn as four pieces round the fighter and one on
 * it.  BTLP only.
 *   0x800BEBCC BtlFxStart90
 *
 * A start handler out of g_btl_spell_fx, and the one seven of the moves in
 * fxspell92.c borrow. Five records built off the fighter aimed at, standing
 * where g_btl_fx_spread's first five entries put them: the four corners of a
 * box round the fighter, and then the fighter's own position. The one on the
 * fighter is made last, is the head of the chain and is the one answered, and
 * it is the only one that arrives at once - the four corners are held six
 * frames.
 *
 * Nothing is passed as the record to hang each new one behind: the chain is
 * threaded by hand through `attached` instead, which is what BtlFxStep uses to
 * find the head again.
 */
#include <decomp/types.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/object.h>
#include <persona/btlp/spellfx.h>

/* Records in the build, and how long the corners are held. */
#define FX_90_PIECES 5
#define FX_90_HOLD   6

BtlObj *BtlFxStart90(void)
{
    BtlObj *o;
    BtlObj *after;
    BtlObj *on;
    long    pos[3];
    int     i;

    g_btl_fx_def.scripts = ((const u_long ***)g_btl_unused_gfx)[0];
    i = FX_90_PIECES - 1;
    after = 0;
    on = g_btl_actors[g_btl_fx_target].obj;
    do {
        pos[0] = on->x + g_btl_fx_spread[i][0];
        pos[1] = on->y + g_btl_fx_spread[i][1];
        pos[2] = 0;
        o = BtlObjAlloc(&g_btl_fx_def, FX_OBJ_GROUP, 0, FX_OBJ_DRAW, 0, pos,
                        FX_OBJ_CD, FX_OBJ_CE);
        o->attr |= FX_OBJ_ATTR;
        if (i != FX_90_PIECES - 1) {
            o->timer = FX_90_HOLD;
        } else {
            o->timer = 0;
        }
        o->attached = after;
        after = o;
        o->mark_num = i + FX_MARK_HEAD;
        i--;
    } while (i >= 0);
    return o;
}
