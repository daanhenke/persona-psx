/* Persona 1 (JP) - the sequencer's message window on screen.  BTLP only.
 *   0x8007C664 BtlSeqWindowDraw
 *
 * While the window has glyphs staged, the frame gets a draw area over the
 * whole working area, one 16x16 sprite per glyph, the draw mode naming the
 * page the glyphs were uploaded to, and a second draw area that cuts
 * everything drawn after it down to the band the window occupies.
 *
 * Each glyph sits in VRAM on a grid of BTL_GLYPH_ROW to a row, in the order it
 * was staged, and takes its palette from the row of the window's CLUT block
 * its cell's attribute names. On screen it goes where its cell says, moved by
 * the window's own position and by its nudge.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <decomp/libc.h>
#include <libgte.h>
#include <libgpu.h>
#include <persona/btlp/draw.h>
#include <persona/btlp/window.h>

/* A glyph's side, on screen and in VRAM. */
#define SEQ_GLYPH 0x10

/* The band the window's text is clipped to, below its nudge. */
#define SEQ_BAND_H 0x30

/* Two areas per display page: the whole working area, and the window's band. */
extern RECT g_btl_seq_clip[][2];

/* Not matched yet (96%): the glyph loop and the draw mode are the image's, and
   so is every register. What is left is the order the two draw areas' stores
   come out in - gcc's second scheduling pass lifts the y load and its store
   ahead of the x store, where the image stores y last, in the call's delay
   slot. The source order below is already x, w, h, y; no reordering of the
   stores, of the mode's bump or of the pointer variables moves it. */
#ifdef NON_MATCHING
void BtlSeqWindowDraw(void)
{
    BtlWindowCell *cell;
    RECT          *clip;
    RECT          *band;
    SPRT_16        glyph;
    DR_MODE        mode;
    DR_AREA        area;
    int            i;

    cell = g_btl_seq_window.cells;
    if (g_btl_seq_window.staged != 0) {
        clip = &g_btl_seq_clip[g_btl_ot_index][0];
        clip->x = g_btl_draw_x;
        clip->w = BTL_AREA_W;
        clip->h = BTL_AREA_H;
        clip->y = g_btl_draw_y;
        SetDrawArea(&area, clip);
        memcpy(g_btl_prim_next, &area, sizeof(DR_AREA));
        AddPrim(g_btl_ot[g_btl_ot_index], g_btl_prim_next);
        g_btl_prim_next += sizeof(DR_AREA);

        SetDrawMode(&mode, 0, 0, GetTPage(0, 0, BTL_SEQ_VRAM_X, BTL_SEQ_VRAM_Y), 0);
        for (i = 0; i < g_btl_seq_window.staged; cell++, i++) {
            SetSprt16(&glyph);
            SetSemiTrans(&glyph, 0);
            SetShadeTex(&glyph, 1);
            glyph.x0 = g_btl_seq_window.dx + g_btl_seq_window.x
                     + cell->x;
            glyph.y0 = g_btl_seq_window.dy + g_btl_seq_window.y
                     + cell->y;
            glyph.u0 = (i % BTL_GLYPH_ROW) * SEQ_GLYPH;
            glyph.v0 = (i / BTL_GLYPH_ROW) * SEQ_GLYPH;
            glyph.clut = GetClut(BTL_SEQ_VRAM_X,
                                 cell->attr + BTL_SEQ_CLUT_Y);
            memcpy(g_btl_prim_next, &glyph, sizeof(SPRT_16));
            AddPrim(g_btl_ot[g_btl_ot_index], g_btl_prim_next);
            g_btl_prim_next += sizeof(SPRT_16);
        }
        memcpy(g_btl_prim_next, &mode, sizeof(DR_MODE));
        AddPrim(g_btl_ot[g_btl_ot_index], g_btl_prim_next);
        band = &g_btl_seq_clip[g_btl_ot_index][1];
        band->x = g_btl_draw_x;
        g_btl_prim_next += sizeof(DR_MODE);
        band->y = g_btl_seq_window.dy + g_btl_draw_y;
        band->w = BTL_AREA_W;
        band->h = SEQ_BAND_H;
        SetDrawArea(&area, band);
        memcpy(g_btl_prim_next, &area, sizeof(DR_AREA));
        AddPrim(g_btl_ot[g_btl_ot_index], g_btl_prim_next);
        g_btl_prim_next += sizeof(DR_AREA);
    }
}
#else
INCLUDE_ASM("btlp/nonmatchings/seqwindowdraw", BtlSeqWindowDraw);
#endif
