/* Persona 1 (JP) - placing an object in the world and drawing it flat.
 *   0x800888F0 BtlDrawObjFlat    (BTLP only)
 *
 * Kind 4 of g_btl_obj_draw. Unlike the other flat kinds it does go through the
 * GTE, but only to find out where the object is: its position is put through
 * the camera, then the GTE is switched over to the object's own matrix and the
 * origin projected, which gives one screen point and one depth. The cells are
 * then laid out flat around that point as ordinary sprites.
 *
 * The depth decides which entry of the ordering table the sprites go into, so
 * an object further away is drawn behind a nearer one - unless it asks not to
 * be sorted, in which case it goes in the single entry at the end.
 *
 * The object's shift is applied in one of two places: to the world position
 * before the camera, or to the screen point afterwards, and one attribute bit
 * says which.
 *
 * BtlDrawObjModel is the kind 5 above it and draws the same object when the
 * intro is still scaling.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <libgte.h>
#include <libgpu.h>
#include <persona/btlp/object.h>
#include <persona/btlp/battle.h>

/* The object's own semi-transparency bit, and where it lands in a primitive. */
#define BTL_OBJ_SEMITRANS 1
#define SPRT_SEMITRANS    2

/* Clear, and the shift moves the object in the world before the camera sees
   it; set, and it moves the projected point instead. */
#define BTL_OBJ_SHIFT_SCREEN 0x40000

/* Set to keep the object out of the depth sort and put it in the one entry at
   the end of the table. */
#define BTL_OBJ_NO_DEPTH 2

/* Where the screen's centre is, for the projection. */
#define BTL_SCREEN_CX 0xA0
#define BTL_SCREEN_CY 0x78

/* Where the ordering table starts from, and the bias a group 2 object takes. */
#define BTL_ORDER_BASE 500
#define BTL_ORDER_MARK 498
#define BTL_GROUP_MARK 2

/* Each frame owns half the primitive pool, and its ordering table sits at the
   end of it. */
#define BTL_FRAME_BYTES 0xE660
#define BTL_PRIM_OT     0xE65C

extern SPRT    *g_btl_sprt_next;
extern u_short  g_btl_tpage[];
extern u_short  g_btl_sprite_count;
extern int      g_btl_screen_dist;
extern MATRIX   g_btl_cam_matrix;
extern MATRIX   g_btl_obj_matrix;
extern SVECTOR  g_btl_obj_quad[];

#ifdef NON_MATCHING
void BtlDrawObjFlat(BtlObj *o)
{
    const BtlGfxCell *cell;
    u_long *ot;
    u_int   i;
    int     otz;
    int     order;
    int     off;
    /* The projected point is one DVECTOR and the two things the
       projection leaves behind are one scratch: the depth in [0], the
       clipping flag in [1]. gcc 2.6 gives an aggregate a lower slot than
       any scalar, so locals written as plain shorts and longs come out in
       the opposite order to the original's frame. */
    DVECTOR sxy;
    short  *py;
    long    scratch[4];

    /* The whole part of the object's 16.16 position. The shift below is read
       as a halfword instead - only these three want the shift. */
    g_btl_obj_quad[0].vx = o->x >> 16;
    g_btl_obj_quad[0].vy = o->y >> 16;
    g_btl_obj_quad[0].vz = o->z >> 16;
    if ((o->attr & BTL_OBJ_SHIFT_SCREEN) == 0) {
        g_btl_obj_quad[0].vz += o->shift >> 16;
    }
    SetGeomOffset(BTL_SCREEN_CX, BTL_SCREEN_CY);
    SetRotMatrix(&g_btl_cam_matrix);
    SetTransMatrix(&g_btl_cam_matrix);
    /* Straight into the object matrix's own translation, so projecting the
       origin below lands where the camera put the object. */
    RotTrans(&g_btl_obj_quad[0], (VECTOR *)g_btl_obj_matrix.t, &scratch[0]);
    /* Placed by the camera, oriented by itself: the origin is what gets
       projected, so the answer is where the object sits on screen. */
    SetRotMatrix(&g_btl_obj_matrix);
    SetTransMatrix(&g_btl_obj_matrix);
    g_btl_obj_quad[0].vx = 0;
    g_btl_obj_quad[0].vy = 0;
    g_btl_obj_quad[0].vz = 0;
    otz = RotTransPers(&g_btl_obj_quad[0], (long *)&sxy, &scratch[1], &scratch[0]);
    order = otz - g_btl_screen_dist / 4;
    off = order + BTL_ORDER_BASE;
    if (o->group == BTL_GROUP_MARK) {
        off = order + BTL_ORDER_MARK;
    }
    cell = ((const BtlGfxList *)o->last)->cells;
    if ((o->attr & BTL_OBJ_SHIFT_SCREEN) != 0) {
        /* Read signed: the shift's whole half carries a sign the projected
           point has to keep. */
        /* Both halves go through the counter, which is not live yet: that is
           what puts them in the registers the original uses. */
        i = *(signed short *)((char *)&o->shift_x + 2);
        sxy.vx = sxy.vx + i;
        i = *(signed short *)((char *)&o->shift + 2);
        sxy.vy = sxy.vy + i;
    }
    if ((o->attr & BTL_OBJ_NO_DEPTH) != 0) {
        ot = (u_long *)(g_btl_prim_pool + g_btl_frame * BTL_FRAME_BYTES
                        + BTL_PRIM_OT);
    } else {
        ot = (u_long *)(g_btl_prim_pool + g_btl_frame * BTL_FRAME_BYTES
                        + BTL_FRAME_BYTES - off * 4);
    }
    i = 0;
    /* The list is reached through the object every time rather than held in a
       local: the count is re-read from it on each turn of the loop. */
    if (*(const u_char *)o->last != 0) {
        do {
            if ((o->attr & BTL_OBJ_SEMITRANS) != 0) {
                g_btl_sprt_next->code |= SPRT_SEMITRANS;
            } else {
                g_btl_sprt_next->code &= ~SPRT_SEMITRANS;
            }
            g_btl_sprt_next->x0 = sxy.vx + cell->x;
            /* Read back through a pointer: that is what keeps the projected y
               in the register the original uses across the loop. */
            py = &sxy.vy;
            g_btl_sprt_next->y0 = *py + cell->y;
            g_btl_sprt_next->u0 = cell->u;
            g_btl_sprt_next->v0 = cell->v;
            g_btl_sprt_next->w = cell->w;
            g_btl_sprt_next->h = cell->h;
            g_btl_sprt_next->r0 = o->rgb[0];
            g_btl_sprt_next->g0 = o->rgb[1];
            cell++;
            g_btl_sprt_next->b0 = o->rgb[2];
            i++;
            g_btl_sprt_next->clut = g_btl_clut[o->unkCE];
            addPrim(ot, g_btl_sprt_next);
            g_btl_sprt_next++;
            g_btl_sprite_count++;
        } while (i < *(const u_char *)o->last);
    }
    /* The list picks the texture page too, three bits of it. */
    SetDrawMode(g_btl_drmode_next, 0, 0,
                g_btl_tpage[o->unkCD + (((const u_short *)o->last)[1] & 7)], 0);
    addPrim(ot, g_btl_drmode_next);
    g_btl_drmode_next++;
}
#else
INCLUDE_ASM("btlp/nonmatchings/objflat", BtlDrawObjFlat);
#endif

