/* Persona 1 (JP) - drawing an object's rectangles through the camera.
 *   0x80089544 BtlDrawObjTilesRot    (BTLP only)
 *
 * Kind 7 of g_btl_obj_draw, the transformed half of the pair BtlDrawObjTiles
 * opens. It draws the same cell list, but instead of laying flat rectangles on
 * the screen it builds a matrix from the object's own rotation and scale, then
 * projects each cell's four corners through it into a POLY_F4.
 *
 * BtlDrawObjects takes the flat kind only while the intro is done scaling, so
 * in a still battle this one never runs and its twin does the same drawing for
 * nothing.
 */
#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <persona/btlp/object.h>
#include <persona/btlp/battle.h>

/* The object's own semi-transparency bit, and where it lands in a primitive. */
#define BTL_OBJ_SEMITRANS 1
#define POLY_SEMITRANS    2

/* Each frame owns half the primitive pool, and its ordering table sits at the
   end of it. */
#define BTL_FRAME_BYTES 0xE660
#define BTL_PRIM_OT     0xE65C

extern POLY_F4 *g_btl_polyf4_next;
extern u_short  g_btl_tpage[];
extern MATRIX   g_btl_obj_matrix;
extern VECTOR   g_btl_obj_shift;
extern SVECTOR  g_btl_obj_quad[];

void BtlDrawObjTilesRot(BtlObj *o)
{
    const BtlGfxCell *cell;
    u_long *ot;
    u_int   i;
    /* One three-long scratch for the projection's two out-parameters,
       the same way objpiecesrot.c has it: gcc 2.6 gives an aggregate the
       lower slot, so two separate locals land the wrong way round. */
    long    scratch[3];

    /* The rotation is the vector of shorts at +0x70 and the scale the three
       words at +0x78. */
    RotMatrix((SVECTOR *)&o->unk70, &g_btl_obj_matrix);
    /* The block boundary is load-bearing: it is what puts the counter in the
       register the original uses. Do not unwrap it. */
    do {
        ScaleMatrix(&g_btl_obj_matrix, (VECTOR *)&o->scale_x);
        TransMatrix(&g_btl_obj_matrix, &g_btl_obj_shift);
        i = 0;
        SetGeomOffset(g_btl_obj_x, g_btl_obj_y);
    } while (0);
    SetRotMatrix(&g_btl_obj_matrix);
    SetTransMatrix(&g_btl_obj_matrix);
    cell = ((const BtlGfxList *)o->last)->cells;
    ot = (u_long *)(g_btl_prim_pool + g_btl_frame * BTL_FRAME_BYTES
                    + BTL_PRIM_OT);
    /* The list is reached through the object every time rather than held in a
       local: the count is re-read from it on each turn of the loop. */
    if (((const BtlGfxList *)o->last)->count != 0) {
        do {
            g_btl_obj_quad[0].vx = cell->x;
            g_btl_obj_quad[0].vy = cell->y;
            g_btl_obj_quad[1].vx = cell->w + cell->x;
            g_btl_obj_quad[1].vy = cell->y;
            g_btl_obj_quad[2].vx = cell->x;
            g_btl_obj_quad[2].vy = cell->h + cell->y;
            g_btl_obj_quad[3].vx = g_btl_obj_quad[1].vx;
            g_btl_obj_quad[3].vy = g_btl_obj_quad[2].vy;
            RotTransPers4(&g_btl_obj_quad[0], &g_btl_obj_quad[1],
                          &g_btl_obj_quad[2], &g_btl_obj_quad[3],
                          (long *)&g_btl_polyf4_next->x0,
                          (long *)&g_btl_polyf4_next->x1,
                          (long *)&g_btl_polyf4_next->x2,
                          (long *)&g_btl_polyf4_next->x3, &scratch[0], &scratch[1]);
            if ((o->attr & BTL_OBJ_SEMITRANS) != 0) {
                g_btl_polyf4_next->code |= POLY_SEMITRANS;
            } else {
                g_btl_polyf4_next->code &= ~POLY_SEMITRANS;
            }
            g_btl_polyf4_next->r0 = o->rgb[0];
            g_btl_polyf4_next->g0 = o->rgb[1];
            g_btl_polyf4_next->b0 = o->rgb[2];
            SetDrawMode(g_btl_drmode_next, 0, 0, g_btl_tpage[o->unkCD], 0);
            cell++;
            i++;
            addPrim(ot, g_btl_polyf4_next);
            addPrim(ot, g_btl_drmode_next);
            g_btl_polyf4_next++;
            g_btl_drmode_next++;
        } while (i < ((const BtlGfxList *)o->last)->count);
    }
}

