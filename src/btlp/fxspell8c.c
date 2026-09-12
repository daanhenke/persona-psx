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
#include <decomp/include_asm.h>
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

/* The row offset and the counter come out in each other's saved registers,
   which drags the argument set-up and the step with them. Six orderings of the
   three set-ups and two spellings of the offset as a compiler giv all leave
   the pair swapped, so this is the permuter's kind of residual rather than a
   shape one. */
#ifdef NON_MATCHING
BtlObj *BtlFxStart8C(void)
{
    BtlObj *o;
    BtlObj *after;
    long    pos[3];
    long    x;
    long    y;
    int     i;

    g_btl_fx_def.scripts = ((const u_long ***)g_btl_unused_gfx)[0];
    i = FX_8C_PIECES - 1;
    after = 0;
    x = FX_8C_X0;
    do {
        if (g_btl_actor_turn >= BTL_PARTY) {
            y = -FX_8C_Y;
        } else {
            y = FX_8C_Y;
        }
        pos[0] = x;
        pos[1] = y;
        pos[2] = 0;
        o = BtlObjAlloc(&g_btl_fx_def, FX_OBJ_GROUP, after, FX_OBJ_DRAW, 0,
                        pos, FX_OBJ_CD, FX_OBJ_CE);
        x -= FX_8C_DX;
        o->attached = after;
        after = o;
        o->mark_num = i;
        o->attr |= FX_OBJ_ATTR;
        i--;
    } while (i >= 0);
    return o;
}
#else
INCLUDE_ASM("btlp/nonmatchings/fxspell8c", BtlFxStart8C);
#endif
