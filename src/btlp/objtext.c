/* Persona 1 (JP) - drawing an object as lines of text.  BTLP only.
 *   0x800881AC BtlDrawObjText
 *
 * Kind 2 of g_btl_obj_draw, reached from BtlDrawObjects. Where the sprite kinds
 * find a list of cells in BtlObj.last, this one finds a list of lines: each
 * names a run of character codes and where on screen to start it.
 *
 * A line ends at its own count or at the first 0xFF, whichever comes first, and
 * every code becomes one 8 wide glyph, the pen stepping 8 to the right. The
 * font is 31 glyphs across, so a code's column is its remainder and its row the
 * quotient, and the line's own v says which row of the page the font starts at.
 *
 * One drawing mode covers the whole object rather than one per glyph, so it is
 * set and linked once at the end.
 *
 * BtlDrawObjTextRot is the kind 3 above it and draws the same lines through the
 * matrix pipeline as transformed quads instead.
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

void BtlDrawObjText(BtlObj *o)
{
    const BtlGfxText *line;
    const u_char *p;
    char    width;
    RECT    tw;         /* declared and never used; the frame is 8 bytes
                           larger than the code needs, so leave it */
    u_long *ot;
    u_int   i;
    int     n;

    i = 0;
    line = (const BtlGfxText *)((const BtlGfxList *)o->last)->cells;
    ot = (u_long *)(g_btl_prim_pool + g_btl_frame * BTL_FRAME_BYTES
                    + BTL_SPRT_OT);
    /* The list is reached through the object every time rather than held in a
       local: the count is re-read from it on each turn of the loop. */
    if (((const BtlGfxList *)o->last)->count != 0) {
        do {
            p = line->text;
            for (n = 0; n < line->count; n++) {
                    if (*p == BTL_TEXT_END) {
                        break;
                    }
                    if ((o->attr & BTL_OBJ_SEMITRANS) != 0) {
                        g_btl_sprt_next->code |= SPRT_SEMITRANS;
                    } else {
                        g_btl_sprt_next->code &= ~SPRT_SEMITRANS;
                    }
                    /* The cell width goes through a local, shared by the pen
                       step, the column and the sprite's own width. */
                    width = BTL_FONT_W;
                    g_btl_sprt_next->x0 = g_btl_obj_x + line->x + n * width;
                    g_btl_sprt_next->y0 = g_btl_obj_y + line->y;
                    g_btl_sprt_next->u0 = (*p % BTL_FONT_COLS) * width;
                    g_btl_sprt_next->v0 = (*p / BTL_FONT_COLS) * BTL_FONT_H
                                          + line->v;
                    g_btl_sprt_next->w = width;
                    g_btl_sprt_next->h = line->h;
                    g_btl_sprt_next->r0 = o->rgb[0];
                    g_btl_sprt_next->g0 = o->rgb[1];
                    g_btl_sprt_next->b0 = o->rgb[2];
                    g_btl_sprt_next->clut = g_btl_clut[line->clut];
                    addPrim(ot, g_btl_sprt_next);
                    g_btl_sprt_next++;
                    g_btl_sprite_count++;
                    p++;
            }
            line++;
            i++;
        } while (i < ((const BtlGfxList *)o->last)->count);
    }
    SetDrawMode(g_btl_drmode_next, 0, 0, g_btl_tpage[o->unkCD], 0);
    /* Not addPrim: the table's head is read out first, and into the counter,
       which is finished with by here. A local of its own does not do - it is
       reusing this one that puts the value in the register the original has. */
    i = getaddr(ot);
    setaddr(g_btl_drmode_next, i);
    setaddr(ot, g_btl_drmode_next);
    g_btl_drmode_next++;
}
