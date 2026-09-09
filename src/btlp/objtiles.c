/* Persona 1 (JP) - drawing an object as plain rectangles.  BTLP only.
 *   0x80089300 BtlDrawObjTiles
 *
 * Kind 6 of g_btl_obj_draw, reached from BtlDrawObjects. The object's current
 * script step leaves a cell list in BtlObj.last: a count and a run of eight
 * byte records, each giving a corner and a size. Kind 6 wants no texture, so
 * only the corner and the size are read and each cell becomes one TILE in the
 * object's own colour.
 *
 * The tile and the drawing mode that carries the object's texture page both go
 * into the frame's ordering table, the mode last so the GPU meets it first.
 *
 * BtlDrawObjTilesRot is the kind 7 above it and draws the same list through
 * the matrix pipeline instead.
 */
#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <persona/btlp/object.h>
#include <persona/btlp/battle.h>

/* The object's own semi-transparency bit, and where it lands in a primitive. */
#define BTL_OBJ_SEMITRANS 1
#define TILE_SEMITRANS    2

/* Each frame owns half the primitive pool, and its ordering table sits at the
   end of it. */
#define BTL_FRAME_BYTES 0xE660
#define BTL_TILE_OT     0xE65C

extern TILE    *g_btl_tile_next;
extern u_short  g_btl_tpage[];

void BtlDrawObjTiles(BtlObj *o)
{
    const BtlGfxCell *cell;
    RECT    tw;         /* declared and never used; the frame is 8 bytes
                           larger than the code needs, so leave it */
    u_long *ot;
    u_int   i;

    /* The list is reached through the object every time rather than held in a
       local: the count is re-read from it on each turn of the loop. */
    cell = ((const BtlGfxList *)o->last)->cells;
    if (((const BtlGfxList *)o->last)->count != 0) {
        i = 0;
        do {
            if ((o->attr & BTL_OBJ_SEMITRANS) != 0) {
                g_btl_tile_next->code |= TILE_SEMITRANS;
            } else {
                g_btl_tile_next->code &= ~TILE_SEMITRANS;
            }
            g_btl_tile_next->x0 = g_btl_obj_x + cell->x;
            g_btl_tile_next->y0 = g_btl_obj_y + cell->y;
            g_btl_tile_next->w = cell->w;
            g_btl_tile_next->h = cell->h;
            g_btl_tile_next->r0 = o->rgb[0];
            g_btl_tile_next->g0 = o->rgb[1];
            g_btl_tile_next->b0 = o->rgb[2];
            SetDrawMode(g_btl_drmode_next, 0, 0, g_btl_tpage[o->unkCD], 0);
            cell++;
            i++;
            ot = (u_long *)(g_btl_prim_pool + g_btl_frame * BTL_FRAME_BYTES
                            + BTL_TILE_OT);
            addPrim(ot, g_btl_tile_next);
            addPrim(ot, g_btl_drmode_next);
            g_btl_tile_next++;
            g_btl_drmode_next++;
        } while (i < ((const BtlGfxList *)o->last)->count);
    }
}
