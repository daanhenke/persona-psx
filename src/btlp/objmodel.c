/* Persona 1 (JP) - drawing an object as a model in the world.
 *   0x80088D44 BtlDrawObjModel    (BTLP only)
 *
 * Kind 5 of g_btl_obj_draw, the transformed half of the pair BtlDrawObjFlat
 * opens, and the one BtlDrawObjects falls back to whenever the intro is still
 * scaling.
 *
 * It projects twice. First the object's own position goes through the camera
 * and is projected from the origin, which answers where on screen the object
 * sits and how far away it is. That screen point, with the screen distance as
 * its depth, then becomes the translation of a second matrix built from the
 * object's rotation and scale - so the cells are laid out around the object in
 * its own frame, and the whole thing lands where the camera put it.
 *
 * The depth picks the ordering table entry, pulled toward the camera by a
 * quarter of the screen distance and pushed out again by a fixed bias; a group
 * 2 object takes a slightly smaller one, which is what keeps a marker behind
 * what it belongs to.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <libgte.h>
#include <libgpu.h>
#include <persona/btlp/object.h>
#include <persona/btlp/battle.h>

/* The object's own semi-transparency bit, and where it lands in a primitive. */
#define BTL_OBJ_SEMITRANS 1
#define POLY_SEMITRANS    2

/* Take the camera's rotation instead of the object's, so the model always
   faces front however the camera has swung. */
#define BTL_OBJ_FACE_CAMERA 8

/* Draw into the arena's ordering table rather than the depth-sorted one. */
#define BTL_OBJ_ARENA_OT 4

/* Clear, and the shift moves the object in the world before the camera sees
   it; set, and it moves each cell in the object's own frame. */
#define BTL_OBJ_SHIFT_SCREEN 0x40000

/* Where the screen's centre is, for the first projection. */
#define BTL_SCREEN_CX 0xA0
#define BTL_SCREEN_CY 0x78

/* Where the ordering table starts from, and the bias a group 2 object takes. */
#define BTL_ORDER_BASE 500
#define BTL_ORDER_MARK 498
#define BTL_GROUP_MARK 2

/* Each frame owns half the primitive pool; the arena's table sits inside it. */
#define BTL_FRAME_BYTES 0xE660
#define BTL_ARENA_OT    0xD6C0

extern u_short   g_btl_poly_count;
extern int       g_btl_screen_dist;
extern MATRIX    g_btl_cam_matrix;
extern MATRIX    g_btl_obj_matrix;
extern SVECTOR   g_btl_cam_rot;
extern SVECTOR   g_btl_obj_quad[];
extern VECTOR    g_btl_intro_x;

#ifdef NON_MATCHING
void BtlDrawObjModel(BtlObj *o)
{
    const BtlGfxCell *cell;
    u_long *ot;
    u_int   i;
    int     otz;
    int     order;
    int     off;
    /* The projected point is one DVECTOR: gcc 2.6 gives an aggregate a
       lower slot than any scalar, so a pair of plain shorts lands above
       the scratch instead of below it. */
    DVECTOR sxy;
    VECTOR  at;
    /* One four-long scratch for what the projections leave behind: the
       clipping flag in [0] and the interpolation value from [1] on. Two
       separate locals put them in the wrong slots - gcc 2.6 gives an
       aggregate the lower address whatever order they are declared in. */
    long    scratch[4];

    /* The whole part of the object's 16.16 position. */
    g_btl_obj_quad[0].vx = o->x >> 16;
    g_btl_obj_quad[0].vy = o->y >> 16;
    g_btl_obj_quad[0].vz = o->z >> 16;
    if ((o->attr & BTL_OBJ_SHIFT_SCREEN) == 0) {
        g_btl_obj_quad[0].vz += o->shift >> 16;
    }
    SetGeomOffset(BTL_SCREEN_CX, BTL_SCREEN_CY);
    SetRotMatrix(&g_btl_cam_matrix);
    SetTransMatrix(&g_btl_cam_matrix);
    /* Straight into the object matrix's own translation. */
    RotTrans(&g_btl_obj_quad[0], (VECTOR *)g_btl_obj_matrix.t, &scratch[0]);
    SetRotMatrix(&g_btl_obj_matrix);
    SetTransMatrix(&g_btl_obj_matrix);
    cell = ((const BtlGfxList *)o->last)->cells;
    g_btl_obj_quad[0].vx = 0;
    g_btl_obj_quad[0].vy = 0;
    g_btl_obj_quad[0].vz = 0;
    otz = RotTransPers(&g_btl_obj_quad[0], (long *)&sxy, &scratch[1], &scratch[0]);
    order = otz - g_btl_screen_dist / 4;
    off = order + BTL_ORDER_BASE;
    /* Taken here rather than at the call below: where the pointer is
       formed decides the registers. */
    if (o->group == BTL_GROUP_MARK) {
        off = order + BTL_ORDER_MARK;
    }
    /* Where the object landed on screen, at the screen's own distance: the
       translation the model itself is then built around. */
    at.vx = sxy.vx;
    at.vy = sxy.vy;
    at.vz = g_btl_screen_dist;
    if ((o->attr & BTL_OBJ_FACE_CAMERA) != 0) {
        o->rot.vx = g_btl_cam_rot.vx;
    }
    RotMatrix(&o->rot, &g_btl_obj_matrix);
    ScaleMatrix(&g_btl_obj_matrix, (VECTOR *)&o->scale_x);
    ScaleMatrix(&g_btl_obj_matrix, &g_btl_intro_x);
    TransMatrix(&g_btl_obj_matrix, &at);
    SetGeomOffset(0, 0);
    SetRotMatrix(&g_btl_obj_matrix);
    SetTransMatrix(&g_btl_obj_matrix);
    i = 0;
    /* The list is reached through the object every time rather than held in a
       local: the count is re-read from it on each turn of the loop. */
    if (*(const u_char *)o->last != 0) {
        do {
            if ((o->attr & BTL_OBJ_SHIFT_SCREEN) == 0) {
                g_btl_obj_quad[0].vx = cell->x;
                g_btl_obj_quad[0].vy = cell->y;
                g_btl_obj_quad[3].vx = cell->w + cell->x;
                g_btl_obj_quad[1].vy = cell->y;
                g_btl_obj_quad[2].vx = cell->x;
                g_btl_obj_quad[2].vy = cell->h + cell->y;
            } else {
                g_btl_obj_quad[0].vx = cell->x + ((short *)&o->shift_x)[1];
                g_btl_obj_quad[0].vy = cell->y + ((short *)&o->shift)[1];
                g_btl_obj_quad[3].vx = cell->w + g_btl_obj_quad[0].vx;
                g_btl_obj_quad[2].vy = cell->h + g_btl_obj_quad[0].vy;
                g_btl_obj_quad[1].vy = g_btl_obj_quad[0].vy;
                g_btl_obj_quad[2].vx = g_btl_obj_quad[0].vx;
            }
            g_btl_obj_quad[1].vx = g_btl_obj_quad[3].vx;
            g_btl_obj_quad[3].vy = g_btl_obj_quad[2].vy;
            RotTransPers4(&g_btl_obj_quad[0], &g_btl_obj_quad[1],
                          &g_btl_obj_quad[2], &g_btl_obj_quad[3],
                          (long *)&g_btl_polyft4_next->x0,
                          (long *)&g_btl_polyft4_next->x1,
                          (long *)&g_btl_polyft4_next->x2,
                          (long *)&g_btl_polyft4_next->x3, &scratch[1], &scratch[0]);
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
            if ((o->attr & BTL_OBJ_SEMITRANS) != 0) {
                g_btl_polyft4_next->code |= POLY_SEMITRANS;
            } else {
                g_btl_polyft4_next->code &= ~POLY_SEMITRANS;
            }
            g_btl_polyft4_next->r0 = o->rgb[0];
            g_btl_polyft4_next->g0 = o->rgb[1];
            g_btl_polyft4_next->b0 = o->rgb[2];
            g_btl_polyft4_next->clut = g_btl_clut[o->unkCE];
            /* The list picks the texture page too, three bits of it. */
            g_btl_polyft4_next->tpage =
                g_btl_tpage[o->unkCD + (((const u_short *)o->last)[1] & 7)];
            if ((o->attr & BTL_OBJ_ARENA_OT) == 0) {
                off = g_btl_frame * BTL_FRAME_BYTES
                      - (off * 4 - BTL_FRAME_BYTES);
            } else {
                /* Two steps on purpose - folding them changes the registers. */
                off = BTL_ARENA_OT;
                off = g_btl_frame * BTL_FRAME_BYTES + off;
            }
            ot = (u_long *)(g_btl_prim_pool + off);
            cell++;
            addPrim(ot, g_btl_polyft4_next);
            g_btl_polyft4_next++;
            g_btl_poly_count++;
            i++;
        } while (i < *(const u_char *)o->last);
    }
}
#else
INCLUDE_ASM("btlp/nonmatchings/objmodel", BtlDrawObjModel);
#endif

