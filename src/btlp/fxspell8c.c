/* Persona 1 (JP) - the move drawn as a row of five pieces across the field.
 * BTLP only.
 *   0x800BEAE8 BtlFxStart8C
 *
 * A start handler out of g_btl_spell_fx. Five records in a row, thirty units
 * apart, walking leftward from sixty units out so the middle one stands on the
 * field's own centre line. The row stands in front of whichever side is
 * acting - twenty units toward the camera for the party and twenty away for
 * the enemies - and each record carries its place in the row as its mark.
 */
#include <decomp/types.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/formation.h>
#include <persona/btlp/object.h>
#include <persona/btlp/round.h>
#include <persona/btlp/spellfx.h>

/* Pieces in the row, where the first one stands and how far apart they are. */
#define FX_8C_PIECES 5
#define FX_8C_X0     (0x3C * PLACE_FIXED)
#define FX_8C_DX     (PLACE_COL_W * PLACE_FIXED)

/* How far in front of the acting side the row stands. */
#define FX_8C_Y (PLACE_ROW_H * PLACE_FIXED)

BtlObj *BtlFxStart8C(void)
{
    BtlObj *o;
    BtlObj *after;
    long    pos[3];
    long    x;
    int     i;

    g_btl_fx_def.scripts = ((const u_long ***)g_btl_unused_gfx)[0];
    i = FX_8C_PIECES - 1;
    after = 0;
    x = FX_8C_X0;
    do {
        if (g_btl_actor_turn >= BTL_PARTY) {
            pos[0] = x;
            pos[1] = -FX_8C_Y;
            pos[2] = 0;
        } else {
            pos[0] = x;
            pos[1] = FX_8C_Y;
            pos[2] = 0;
        }
        o = BtlObjAlloc(&g_btl_fx_def, FX_OBJ_GROUP, after, FX_OBJ_DRAW, 0,
                        pos, FX_OBJ_CD, FX_OBJ_CE);
        o->attr |= FX_OBJ_ATTR;
        o->attached = after;
        after = o;
        x -= FX_8C_DX;
        o->mark_num = i;
        i--;
    } while (i >= 0);
    return o;
}
