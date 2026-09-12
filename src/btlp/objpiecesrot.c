/* Persona 1 (JP) - drawing an object's pieces through the camera.
 *   0x80087E28 BtlDrawObjPiecesRot    (BTLP only)
 *
 * Kind 1 of g_btl_obj_draw, the transformed half of the pair BtlDrawObjPieces
 * opens. It walks the same cell list, but builds a matrix from the object's own
 * rotation and scale and projects each cell's four corners through it into a
 * textured quad.
 *
 * The texture page goes into the quad itself rather than through a drawing
 * mode, so unlike the flat kinds this one links only the one primitive.
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

extern u_short   g_btl_poly_count;
extern MATRIX    g_btl_obj_matrix;
extern VECTOR    g_btl_obj_shift;
extern SVECTOR   g_btl_obj_quad[];

void BtlDrawObjPiecesRot(BtlObj *o)
{
    const BtlGfxCell *cell;
    u_long *ot;
    u_int   i;
    /* The projection's two out-parameters are one three-long scratch:
       the interpolation value in [0] and the clipping flag in [1]. Two
       separate locals do not compile to this - gcc 2.6 gives an
       aggregate the lower slot regardless of declaration order, so a
       long and a long[2] come out the other way round. */
    long    scratch[3];

    /* The rotation is the vector of shorts at +0x70 and the scale the three
       words at +0x78. */
    RotMatrix(&o->rot, &g_btl_obj_matrix);
    ScaleMatrix(&g_btl_obj_matrix, (VECTOR *)&o->scale_x);
    TransMatrix(&g_btl_obj_matrix, &g_btl_obj_shift);
    /* The block boundary is load-bearing: it is what puts the counter in the
       register the original uses. Do not unwrap it. */
    do {
        SetRotMatrix(&g_btl_obj_matrix);
        SetTransMatrix(&g_btl_obj_matrix);
        i = 0;
    } while (0);
    SetGeomOffset(g_btl_obj_x, g_btl_obj_y);
    cell = ((const BtlGfxList *)o->last)->cells;
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
                          (long *)&g_btl_polyft4_next->x0,
                          (long *)&g_btl_polyft4_next->x1,
                          (long *)&g_btl_polyft4_next->x2,
                          (long *)&g_btl_polyft4_next->x3, &scratch[0], &scratch[1]);
            if ((o->attr & BTL_OBJ_SEMITRANS) != 0) {
                g_btl_polyft4_next->code |= POLY_SEMITRANS;
            } else {
                g_btl_polyft4_next->code &= ~POLY_SEMITRANS;
            }
            g_btl_polyft4_next->u0 = cell->u;
            g_btl_polyft4_next->v0 = cell->v;
            g_btl_polyft4_next->u1 = cell->u + cell->w;
            g_btl_polyft4_next->v1 = cell->v;
            g_btl_polyft4_next->u2 = cell->u;
            g_btl_polyft4_next->v2 = cell->v + cell->h;
            /* The far corner takes its two from the quad's own edges rather
               than adding them again. */
            g_btl_polyft4_next->u3 = g_btl_polyft4_next->u1;
            g_btl_polyft4_next->v3 = g_btl_polyft4_next->v2;
            g_btl_polyft4_next->r0 = o->rgb[0];
            cell++;
            g_btl_polyft4_next->g0 = o->rgb[1];
            i++;
            g_btl_polyft4_next->b0 = o->rgb[2];
            g_btl_polyft4_next->clut = g_btl_clut[o->unkCE];
            g_btl_polyft4_next->tpage = g_btl_tpage[o->unkCD];
            ot = (u_long *)(g_btl_prim_pool + g_btl_frame * BTL_FRAME_BYTES
                            + BTL_PRIM_OT);
            addPrim(ot, g_btl_polyft4_next);
            g_btl_polyft4_next++;
            g_btl_poly_count++;
        } while (i < ((const BtlGfxList *)o->last)->count);
    }
}

