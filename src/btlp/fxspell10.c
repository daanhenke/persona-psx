/* Persona 1 (JP) - a sheet that rises off the field, and a step handler
 * nothing reaches.  BTLP only.
 *   0x800B7E2C BtlFxStepUnused2  0x800B7F88 BtlFxStart10
 *
 * BtlFxStart10 lays a five-wide sheet over the side being aimed at and gives
 * every record a standing lift, so the whole sheet drifts up the screen as it
 * plays. How deep the sheet is depends on the move the fighter is making:
 * move 0x11 - the one whose own start handler is nothing but a call to this
 * one - gets three rows, and everything else gets one.
 *
 * BtlFxStepUnused2 in front of it is a step handler of the ordinary shape,
 * four phases and a hit armed at the end, but no record in g_btl_spell_fx
 * points at it, nothing calls it, and no word anywhere in BTLP.BIN holds its
 * address - the same way move 0x0F's second start handler is unreachable. It
 * is here because the assembler laid it down between two that are used.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/formation.h>
#include <persona/btlp/object.h>
#include <persona/btlp/round.h>
#include <persona/btlp/spellfx.h>

/* The two the step handler clears as it starts, and the pair it waits on. */
#define FX_STEP_SETTLE 0x20

/* What the last phase leaves behind: the record is hidden, put on the phase
   nothing steps, and given four children for the hit to count. */
#define FX_STEP_DONE   0x80
#define FX_STEP_HITS   4

/* The move whose sheet is three rows deep rather than one. */
#define FX_10_TALL 0x11
#define FX_10_ROWS 3

/* How far apart the rows stand and where the party's side starts, in whole
   units: two rows of the field to a row of the sheet. */
#define FX_10_ROW_H (2 * PLACE_ROW_H)
#define FX_10_HIGH  0x8C

/* How far every record of the sheet drifts each frame - 16.16, so a whole
   unit, up the screen on the party's side and down on the other. */
#define FX_10_RISE 0x10000

/* How fast the middle phase carries the sheet - 16.16, an eighth of a unit
   added to the drift every frame. */
#define FX_10_LIFT 0x20000

void BtlFxStepUnused2(BtlObj *o)
{
    switch (o->phase) {
    case 0:
        if (o->timer != 0) {
            break;
        }
        o->attr &= ~(BTL_OBJ_HIDDEN | BTL_OBJ_STATIC);
        o->phase++;
        break;
    case 1:
        if ((o->attr & BTL_OBJ_ANIMATING) != 0) {
            break;
        }
        o->timer = FX_STEP_SETTLE;
        o->phase++;
        break;
    case 2:
        o->y += o->step_y;
        o->step_y += (g_btl_actor_turn < BTL_PARTY) ? -FX_10_LIFT
                                                    : FX_10_LIFT;
        if (o->timer != 0) {
            break;
        }
        o->phase++;
        break;
    case 3:
        if (o->mark_num == FX_MARK_HEAD) {
            o->phase = FX_STEP_DONE;
            o->attr |= BTL_OBJ_HIDDEN;
            BtlArmHitChain();
            o->children = FX_STEP_HITS;
        } else {
            BtlObjFree(o);
        }
        break;
    default:
        BtlFinishMoveFx(o);
        break;
    }
}

/* 98.21%: one instruction, and it is which of the two the delay slot in front
   of the loop takes - the image fills it with the row counter's zero and leaves
   the row bound behind it, and gcc here does the opposite whichever order the
   two are written in. */
#ifdef NON_MATCHING
BtlObj *BtlFxStart10(void)
{
    BtlObj *o;
    BtlObj *prev;
    long    pos[3];
    long    x;
    long    y_party;
    long    y_enemy;
    int     rows;
    int     top;
    int     row;
    int     col;
    int     cell;

    g_btl_fx_def.scripts = ((const u_long ***)g_btl_unused_gfx)[0];
    if (g_btl_actors[g_btl_actor_turn].move == FX_10_TALL) {
        rows = FX_10_ROWS;
    } else {
        rows = 1;
    }
    cell = rows * FX_GRID_W - 1;
    prev = 0;
    top  = FX_10_ROWS;
    for (row = 0; row < rows; row++) {
        col     = 0;
        y_party = ((top - row) * FX_10_ROW_H - FX_10_HIGH) * PLACE_FIXED;
        x       = FX_GRID_X0;
        y_enemy = (PLACE_ROW_H + row * FX_10_ROW_H) * PLACE_FIXED;
        for (; col < FX_GRID_W; col++) {
            if (g_btl_actor_turn < BTL_PARTY) {
                pos[0] = x;
                pos[1] = y_party;
                pos[2] = 0;
            } else {
                pos[0] = x;
                pos[1] = y_enemy;
                pos[2] = 0;
            }
            o = BtlObjAlloc(&g_btl_fx_def, FX_OBJ_GROUP, prev, FX_OBJ_DRAW, 0,
                            pos, FX_OBJ_CD, FX_OBJ_CE);
            o->mark_num = cell + FX_MARK_HEAD;
            o->attr     = FX_OBJ_ATTR;
            o->attached = prev;
            o->step_y   = (g_btl_actor_turn < BTL_PARTY) ? -FX_10_RISE
                                                         : FX_10_RISE;
            prev = o;
            x += FX_GRID_DX;
            cell--;
        }
    }
    return o;
}
#else
INCLUDE_ASM("btlp/nonmatchings/fxspell10", BtlFxStart10);
#endif
