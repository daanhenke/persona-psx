/* Persona 1 (JP) - drawing an object as loose sprites.  BTLP only.
 *   0x80087B80 BtlDrawObjPieces
 *
 * Kind 0 of g_btl_obj_draw, reached from BtlDrawObjects. The object's current
 * script step leaves a cell list in BtlObj.last, and each cell becomes one
 * SPRT at the object's screen position, taking its texture corner and size
 * from the cell and its colour and palette from the object.
 *
 * The sprite and the drawing mode that carries the object's texture page both
 * go into the frame's ordering table, the mode last so the GPU meets it first.
 *
 * BtlDrawObjPiecesRot is the kind 1 above it and draws the same list through
 * the matrix pipeline as transformed quads instead.
 */
#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <persona/btlp/object.h>
#include <persona/btlp/battle.h>

/* The object's own semi-transparency bit, and where it lands in a primitive. */
#define BTL_OBJ_SEMITRANS 1
#define SPRT_SEMITRANS    2

/* Each frame owns half the primitive pool, and its ordering table sits at the
   end of it. */
#define BTL_FRAME_BYTES 0xE660
#define BTL_SPRT_OT     0xE65C

extern SPRT    *g_btl_sprt_next;
extern u_short  g_btl_tpage[];
extern u_short  g_btl_sprite_count;

void BtlDrawObjPieces(BtlObj *o)
{
    const BtlGfxCell *cell;
    RECT    tw;         /* declared and never used; the frame is 8 bytes
                           larger than the code needs, so leave it */
    u_long *ot;
    u_int   i;

    i = 0;
    cell = ((const BtlGfxList *)o->last)->cells;
    ot = (u_long *)(g_btl_prim_pool + g_btl_frame * BTL_FRAME_BYTES
                    + BTL_SPRT_OT);
    /* The list is reached through the object every time rather than held in a
       local: the count is re-read from it on each turn of the loop. */
    if (((const BtlGfxList *)o->last)->count != 0) {
        do {
            /* The clear needs a block boundary in front of it, and testing
               something already to hand is how the original gets one: both
               arms below do the same thing on purpose. */
            if ((o->attr & BTL_OBJ_SEMITRANS) != 0) {
                g_btl_sprt_next->code |= SPRT_SEMITRANS;
            } else if (getaddr(ot) || i) {
                if (cell) {
                    g_btl_sprt_next->code &= ~SPRT_SEMITRANS;
                } else {
                    g_btl_sprt_next->code &= ~SPRT_SEMITRANS;
                }
            } else {
                g_btl_sprt_next->code &= ~SPRT_SEMITRANS;
            }
            g_btl_sprt_next->x0 = g_btl_obj_x + cell->x;
            g_btl_sprt_next->y0 = g_btl_obj_y + cell->y;
            g_btl_sprt_next->u0 = cell->u;
            g_btl_sprt_next->v0 = cell->v;
            g_btl_sprt_next->w = cell->w;
            g_btl_sprt_next->h = cell->h;
            g_btl_sprt_next->r0 = o->rgb[0];
            g_btl_sprt_next->g0 = o->rgb[1];
            g_btl_sprt_next->b0 = o->rgb[2];
            g_btl_sprt_next->clut = g_btl_clut[o->unkCE];
            SetDrawMode(g_btl_drmode_next, 0, 0, g_btl_tpage[o->unkCD], 0);
            cell++;
            i++;
            addPrim(ot, g_btl_sprt_next);
            addPrim(ot, g_btl_drmode_next);
            g_btl_sprt_next++;
            /* Between the two increments, not before them: that is where the
               original counts the sprite. */
            g_btl_sprite_count++;
            g_btl_drmode_next++;
        } while (i < ((const BtlGfxList *)o->last)->count);
    }
}
