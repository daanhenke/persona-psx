/* Persona 1 (JP) - drawing an object's own quads and lines.  BTLP only.
 *   0x80089840 BtlDrawObjQuads   0x80089C00 BtlDrawObjLines
 *   0x8008A114 BtlObjPlaceUnused
 *
 * Kinds 8 and 9 of g_btl_obj_draw are BtlDrawObjQuads and kinds 10 and 11
 * BtlDrawObjLines; neither has a flat twin the way the kinds below them do.
 *
 * BtlDrawObjQuads projects each rectangle of the list through a matrix built
 * from the object's rotation, scale and shift into a Gouraud quad, colours
 * its four corners out of the record, and puts the whole run into the frame's
 * last ordering table entry behind one draw mode.
 *
 * BtlDrawObjLines works in one of two ways. With BTL_OBJ_LINES_3D set the
 * object is first placed through the camera, turned to face it, and each line
 * projected from there into the table's first entry; without it the lines are
 * laid flat on the screen around the object's point, taking its
 * semi-transparency, into the last entry.
 *
 * BtlObjPlaceUnused sets up the same placement as a flat object does and draws
 * nothing. Nothing calls it.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <libgte.h>
#include <libgpu.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/object.h>

/* The object's own semi-transparency bit, and where it lands in a primitive. */
#define BTL_OBJ_SEMITRANS 1
#define PRIM_SEMITRANS    2

/* Set to project the lines through the camera rather than lay them flat. */
#define BTL_OBJ_LINES_3D 0x100000

/* Where the screen's centre is, for the projection. */
#define BTL_SCREEN_CX 0xA0
#define BTL_SCREEN_CY 0x78

extern MATRIX  g_btl_cam_matrix;
extern MATRIX  g_btl_obj_matrix;
extern VECTOR  g_btl_obj_shift;
extern SVECTOR g_btl_obj_quad[];

void BtlDrawObjQuads(BtlObj *o)
{
    const BtlGfxQuad *q;
    u_long *ot;
    u_int   i;
    long    scratch[3];

    RotMatrix(&o->rot, &g_btl_obj_matrix);
    ScaleMatrix(&g_btl_obj_matrix, (VECTOR *)&o->scale_x);
    TransMatrix(&g_btl_obj_matrix, &g_btl_obj_shift);
    do {
        i = 0;
        SetGeomOffset(g_btl_obj_x, g_btl_obj_y);
    } while (0);
    SetRotMatrix(&g_btl_obj_matrix);
    SetTransMatrix(&g_btl_obj_matrix);
    ot = (u_long *)(g_btl_prim_pool + g_btl_frame * BTL_FRAME_STRIDE
                    + BTL_OT_END);
    q = ((const BtlGfxQuadList *)o->last)->quads;
    if (((const BtlGfxQuadList *)o->last)->count != 0) {
        do {
            g_btl_obj_quad[0].vx = q->x;
            g_btl_obj_quad[0].vy = q->y;
            g_btl_obj_quad[1].vx = q->x + q->w;
            g_btl_obj_quad[1].vy = q->y;
            g_btl_obj_quad[2].vx = q->x;
            g_btl_obj_quad[2].vy = q->y + q->h;
            g_btl_obj_quad[3].vx = g_btl_obj_quad[1].vx;
            g_btl_obj_quad[3].vy = g_btl_obj_quad[2].vy;
            RotTransPers4(&g_btl_obj_quad[0], &g_btl_obj_quad[1],
                          &g_btl_obj_quad[2], &g_btl_obj_quad[3],
                          (long *)&g_btl_polyg4_next->x0,
                          (long *)&g_btl_polyg4_next->x1,
                          (long *)&g_btl_polyg4_next->x2,
                          (long *)&g_btl_polyg4_next->x3, &scratch[0],
                          &scratch[1]);
            if ((o->attr & BTL_OBJ_SEMITRANS) != 0) {
                g_btl_polyg4_next->code |= PRIM_SEMITRANS;
            } else {
                g_btl_polyg4_next->code &= ~PRIM_SEMITRANS;
            }
            g_btl_polyg4_next->r0 = q->rgb[0];
            g_btl_polyg4_next->g0 = q->rgb[1];
            g_btl_polyg4_next->b0 = q->rgb[2];
            g_btl_polyg4_next->r1 = q->rgb[3];
            g_btl_polyg4_next->g1 = q->rgb[4];
            g_btl_polyg4_next->b1 = q->rgb[5];
            g_btl_polyg4_next->r2 = q->rgb[6];
            g_btl_polyg4_next->g2 = q->rgb[7];
            g_btl_polyg4_next->b2 = q->rgb[8];
            g_btl_polyg4_next->r3 = q->rgb[9];
            g_btl_polyg4_next->g3 = q->rgb[10];
            g_btl_polyg4_next->b3 = q->rgb[11];
            q++;
            i++;
            addPrim(ot, g_btl_polyg4_next);
            g_btl_polyg4_next++;
        } while (i < ((const BtlGfxQuadList *)o->last)->count);
    }
    SetDrawMode(g_btl_drmode_next, 0, 0, g_btl_tpage[o->unkCD], 0);
    addPrim(ot, g_btl_drmode_next);
    g_btl_drmode_next++;
}

/* 99.62%, and only registers are left: the image copies both scratch
   addresses into fresh saved registers ahead of the 3D loop, where this
   copies only one, so the two come out in each other's. The frame, every slot
   and every instruction are the image's. */
#ifdef NON_MATCHING
void BtlDrawObjLines(BtlObj *o)
{
    const BtlGfxLine *l;
    u_int    i;
    DVECTOR  sxy;
    VECTOR   pos;
    /* Sized to the frame the image reserves, not to the two it reads. */
    long     scratch[6];

    l = ((const BtlGfxLineList *)o->last)->lines;
    if ((o->attr & BTL_OBJ_LINES_3D) != 0) {
        g_btl_obj_quad[0].vx = o->x >> 16;
        g_btl_obj_quad[0].vy = o->y >> 16;
        g_btl_obj_quad[0].vz = o->z >> 16;
        SetGeomOffset(BTL_SCREEN_CX, BTL_SCREEN_CY);
        SetRotMatrix(&g_btl_cam_matrix);
        SetTransMatrix(&g_btl_cam_matrix);
        RotTrans(&g_btl_obj_quad[0], (VECTOR *)g_btl_obj_matrix.t, &scratch[0]);
        SetRotMatrix(&g_btl_obj_matrix);
        SetTransMatrix(&g_btl_obj_matrix);
        g_btl_obj_quad[0].vx = 0;
        g_btl_obj_quad[0].vy = 0;
        g_btl_obj_quad[0].vz = 0;
        RotTransPers(&g_btl_obj_quad[0], (long *)&sxy, &scratch[1],
                     &scratch[0]);
        pos.vx = sxy.vx;
        pos.vy = sxy.vy;
        pos.vz = g_btl_screen_dist;
        o->rot.vx = g_btl_cam_rot.vx;
        RotMatrix(&o->rot, &g_btl_obj_matrix);
        ScaleMatrix(&g_btl_obj_matrix, (VECTOR *)&o->scale_x);
        ScaleMatrix(&g_btl_obj_matrix, &g_btl_view_scale);
        TransMatrix(&g_btl_obj_matrix, &pos);
        SetGeomOffset(0, 0);
        SetRotMatrix(&g_btl_obj_matrix);
        SetTransMatrix(&g_btl_obj_matrix);
        i = 0;
        if (((const BtlGfxLineList *)o->last)->count != 0) {
            do {
                g_btl_obj_quad[0].vx = l->x0;
                g_btl_obj_quad[0].vy = l->y0;
                g_btl_obj_quad[1].vx = l->x1;
                g_btl_obj_quad[1].vy = l->y1;
                RotTransPers(&g_btl_obj_quad[0],
                             (long *)&g_btl_lineg2_next->x0, &scratch[1],
                             &scratch[0]);
                RotTransPers(&g_btl_obj_quad[1],
                             (long *)&g_btl_lineg2_next->x1, &scratch[1],
                             &scratch[0]);
                g_btl_lineg2_next->r0 = l->rgb[0];
                g_btl_lineg2_next->g0 = l->rgb[1];
                g_btl_lineg2_next->b0 = l->rgb[2];
                g_btl_lineg2_next->r1 = l->rgb[3];
                g_btl_lineg2_next->g1 = l->rgb[4];
                g_btl_lineg2_next->b1 = l->rgb[5];
                l++;
                i++;
                /* The table's address written into the call: the image
                   loads the primitive's tag before the address is done. */
                addPrim((u_long *)(g_btl_prim_pool
                                   + g_btl_frame * BTL_FRAME_STRIDE + BTL_OT),
                        g_btl_lineg2_next);
                g_btl_lineg2_next++;
            } while (i < ((const BtlGfxLineList *)o->last)->count);
        }
    } else {
        i = 0;
        if (((const BtlGfxLineList *)o->last)->count != 0) {
            do {
                /* The clear wants a block boundary in front of it, the way
                   objpieces.c's does: both arms below do the same thing, and
                   testing the counter is what puts it and the record in the
                   image's registers. */
                if ((o->attr & BTL_OBJ_SEMITRANS) != 0) {
                    g_btl_lineg2_next->code |= PRIM_SEMITRANS;
                } else if (i) {
                    g_btl_lineg2_next->code &= ~PRIM_SEMITRANS;
                } else {
                    g_btl_lineg2_next->code &= ~PRIM_SEMITRANS;
                }
                g_btl_lineg2_next->x0 = g_btl_obj_x + l->x0;
                g_btl_lineg2_next->y0 = g_btl_obj_y + l->y0;
                g_btl_lineg2_next->x1 = g_btl_obj_x + l->x1;
                g_btl_lineg2_next->y1 = g_btl_obj_y + l->y1;
                g_btl_lineg2_next->r0 = l->rgb[0];
                g_btl_lineg2_next->g0 = l->rgb[1];
                g_btl_lineg2_next->b0 = l->rgb[2];
                g_btl_lineg2_next->r1 = l->rgb[3];
                g_btl_lineg2_next->g1 = l->rgb[4];
                g_btl_lineg2_next->b1 = l->rgb[5];
                l++;
                i++;
                addPrim((u_long *)(g_btl_prim_pool
                                   + g_btl_frame * BTL_FRAME_STRIDE + BTL_OT_END),
                        g_btl_lineg2_next);
                g_btl_lineg2_next++;
            } while (i < ((const BtlGfxLineList *)o->last)->count);
        }
    }
}
#else
INCLUDE_ASM("btlp/nonmatchings/objlists", BtlDrawObjLines);
#endif

void BtlObjPlaceUnused(BtlObj *o)
{
    /* Eight bytes the frame reserves below the transform's flag and nothing
       reads - the opening of BtlDrawObjFlat, without the rest. */
    long unused[2];
    long scratch[2];

    g_btl_obj_quad[0].vx = o->x >> 16;
    g_btl_obj_quad[0].vy = o->y >> 16;
    g_btl_obj_quad[0].vz = o->z >> 16;
    if ((o->attr & BTL_OBJ_SHIFT_SCREEN) == 0) {
        g_btl_obj_quad[0].vz += o->shift >> 16;
    }
    SetGeomOffset(BTL_SCREEN_CX, BTL_SCREEN_CY);
    RotMatrix(&o->rot, &g_btl_obj_matrix);
    ScaleMatrix(&g_btl_obj_matrix, (VECTOR *)&o->scale_x);
    ScaleMatrix(&g_btl_obj_matrix, &g_btl_view_scale);
    SetRotMatrix(&g_btl_cam_matrix);
    SetTransMatrix(&g_btl_cam_matrix);
    RotTrans(&g_btl_obj_quad[0], (VECTOR *)g_btl_obj_matrix.t, &scratch[0]);
    SetRotMatrix(&g_btl_obj_matrix);
    SetTransMatrix(&g_btl_obj_matrix);
}
