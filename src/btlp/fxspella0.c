/* Persona 1 (JP) - the move that gathers into a point and bursts.  BTLP only.
 *   0x800BF038 BtlFxStartA0  0x800BF0F8 BtlFxStepA0
 *
 * The start handler opens one record over the acting side, already walking to
 * a mid grey, and gives it the two bits the drawing side wants for a shape
 * that is scaled down to nothing.
 *
 * BtlFxStepA0 is its step handler: five phases behind a jump table. It waits
 * for the grey, shrinks by an eighth of itself a frame until it is nearly
 * gone, throws seventy-five records out of the point - three layers of a
 * five-by-five sheet, each aimed back at where the head stands and timed off
 * the same scatter table the other sheets use - and then holds, darkens and
 * arms the hit.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <persona/btlp/formation.h>
#include <persona/btlp/object.h>
#include <persona/btlp/round.h>
#include <persona/btlp/spellfx.h>

/* How far off the middle of the field the record stands - 16.16, a hundred
   units, on whichever side is acting. */
#define FX_A0_OFF 0x640000

/* The grey it walks to as it opens, and how fast. */
#define FX_A0_GREY 0x80
#define FX_A0_FADE 4

/* The two bits it is given on top of what BtlObjAlloc left. */
#define FX_A0_ATTR 3

/* How fast the point shrinks and how small it gets before it bursts. */
#define FX_A0_EIGHTH 8
#define FX_A0_GONE   9

/* What bursts out of it: three layers of the same five-by-five sheet, and
   where the top row of one starts - 16.16, a hundred and forty units up. */
#define FX_A0_LAYERS 3
#define FX_A0_CELLS  (FX_GRID_W * FX_GRID_H)
#define FX_A0_TOP    (-0x8C0000)

/* What each record that bursts out is given: an eighth of the way to its cell
   each frame, a standing lift toward the camera, and how long it lives. */
#define FX_A0_REACH 8
#define FX_A0_RISE  (-0x40000)
#define FX_A0_LIFE  0x20

/* How long the head holds once the sheet is out. */
#define FX_A0_HOLD 0x3C

/* What the last phase leaves behind. */
#define FX_A0_DONE 0x80

BtlObj *BtlFxStartA0(void)
{
    BtlObj *o;
    long    pos[3];

    if (g_btl_actor_turn < BTL_PARTY) {
        pos[0] = 0;
        pos[1] = -FX_A0_OFF;
        pos[2] = 0;
    } else {
        pos[0] = 0;
        pos[1] = FX_A0_OFF;
        pos[2] = 0;
    }
    g_btl_fx_def.scripts = ((const u_long ***)g_btl_unused_gfx)[0];
    o = BtlObjAlloc(&g_btl_fx_def, FX_OBJ_GROUP, 0, FX_OBJ_DRAW, 0, pos,
                    FX_OBJ_CD, FX_OBJ_CE);
    o->mark_num = FX_MARK_HEAD;
    o->rgb_to[0] = FX_A0_GREY;
    o->rgb_to[1] = FX_A0_GREY;
    o->rgb_to[2] = FX_A0_GREY;
    o->rgb[0] = 0;
    o->rgb[1] = 0;
    o->rgb[2] = 0;
    o->fade = FX_A0_FADE;
    o->attr |= FX_A0_ATTR;
    return o;
}

#ifdef NON_MATCHING
void BtlFxStepA0(BtlObj *o)
{
    BtlObj *n;
    BtlObj *prev;
    long    x;
    long    y;
    int     layer;
    int     row;
    long    y0;
    int     col;
    int     cell;

    switch (o->mark_num) {
    case FX_MARK_HEAD:
        switch (o->phase) {
        case 0:
            if (o->rgb[0] != FX_A0_GREY) {
                return;
            }
            o->phase++;
            return;
        case 1:
            o->scale_x -= o->scale_x / FX_A0_EIGHTH;
            o->scale_y -= o->scale_y / FX_A0_EIGHTH;
            if (o->scale_x >= FX_A0_GONE) {
                return;
            }
            o->phase++;
            return;
        case 2:
            prev = 0;
            g_btl_fx_def.scripts = ((const u_long ***)g_btl_unused_gfx)[1];
            cell = 0;
            layer = 0;
            row = 0;
            do {
                y0 = FX_A0_TOP;
                do {
                    col = 0;
                    y = y0;
                    x = FX_GRID_X0;
                    do {
                        n = BtlObjAlloc(&g_btl_fx_def, FX_OBJ_GROUP, prev,
                                        FX_OBJ_DRAW, 0, &o->x,
                                        FX_OBJ_CD, FX_OBJ_CE);
                        n->mark_num = FX_MARK_REST;
                        n->attached = prev;
                        n->attr |= FX_OBJ_ATTR;
                        n->timer =
                            (g_btl_fx_grid_order[FX_MARK_HEAD
                                                 + cell % FX_A0_CELLS]
                             + cell / FX_A0_CELLS * FX_A0_CELLS * 2) / 2;
                        n->step_x = (x - n->x) / FX_A0_REACH;
                        n->step_y = (y - n->y) / FX_A0_REACH;
                        n->step_z = FX_A0_RISE;
                        n->steps = FX_A0_LIFE;
                        prev = n;
                        x += FX_GRID_DX;
                        col++;
                        cell++;
                    } while (col < FX_GRID_W);
                    y0 += FX_GRID_DY;
                    row++;
                } while (row < FX_GRID_H);
                layer++;
                row = 0;
            } while (layer < FX_A0_LAYERS);
            BtlObjSetMotion(n, o->motion);
            BtlObjSetKind(n, o->kind);
            o->timer = FX_A0_HOLD;
            o->phase++;
            return;
        case 3:
            if (o->timer != 0) {
                return;
            }
            o->rgb_to[0] = 0;
            o->rgb_to[1] = 0;
            o->rgb_to[2] = 0;
            o->phase++;
            return;
        case 4:
            if (o->rgb[0] != 0) {
                return;
            }
            o->phase = FX_A0_DONE;
            o->attr |= BTL_OBJ_HIDDEN;
            o->children = (u_char)g_btl_spell_fx[o->kind].group;
            BtlArmHitChain();
            return;
        default:
            BtlFxFinish01(o);
            return;
        }
    case FX_MARK_REST:
        switch (o->phase) {
        case 0:
            if (o->timer != 0) {
                return;
            }
            o->attr &= ~(BTL_OBJ_HIDDEN | BTL_OBJ_STATIC);
            o->phase++;
            return;
        case 1:
            o->x += o->step_x;
            o->y += o->step_y;
            o->z += o->step_z;
            o->steps--;
            if (o->steps == 0) {
                BtlObjFree(o);
            }
            return;
        default:
            return;
        }
    default:
        return;
    }
}
#else
INCLUDE_ASM("btlp/nonmatchings/fxspella0", BtlFxStepA0);
#endif
