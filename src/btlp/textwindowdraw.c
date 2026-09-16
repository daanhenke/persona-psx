/* Persona 1 (JP) - the second message window on screen.  BTLP only.
 *   0x8007CC60 BtlTextWindowDraw
 *
 * The same job BtlSeqWindowDraw does for the sequencer's window, and the same
 * layout of glyphs in VRAM: a grid of BTL_GLYPH_ROW to a row, in the order
 * they were staged, each taking its palette from the row of the window's CLUT
 * block that its cell's attribute names.
 *
 * This one is drawn two ways, by whether the box behind it is still opening.
 * A box at full size on both axes takes the flat path: a draw area over the
 * whole working area, one 16x16 sprite per glyph, the draw mode naming the
 * page the glyphs went to, and a second draw area that cuts everything drawn
 * after it down to the band the window occupies.
 *
 * A box that has not got there yet takes the other: the glyphs go through the
 * GTE on the box's own matrix, so each is a textured quad with its four
 * corners transformed rather than a sprite, and the whole line grows with the
 * box. Nothing is clipped on that path and the draw mode is left alone, since
 * every quad carries its own page.
 *
 * @note The geometry offset the transformed path sets is read into a pair of
 * locals first and then never put back, so the offset the box's line leaves
 * behind stands until something else sets one.
 */
#include <decomp/types.h>
#include <decomp/libc.h>
#include <libgte.h>
#include <libgpu.h>
#include <persona/btlp/box.h>
#include <persona/btlp/draw.h>
#include <persona/btlp/text.h>
#include <persona/btlp/window.h>

/* A glyph's side, on screen and in VRAM, and where the glyph rows start
   inside the page the quads name - BTL_TEXT_GLYPH_Y less the page's own
   corner. */
#define TEXT_GLYPH   0x10
#define TEXT_GLYPH_V 0x40

/* Where a glyph sits relative to the window, and the band the flat path clips
   the line to. */
#define TEXT_RAISE  8
#define TEXT_BAND_H 0x10

/* The box's rotation and translation, which sit immediately in front of its
   scale. The image reaches both off the scale's own address - the one it has
   already loaded to test the box with - rather than by name, so they are
   written as offsets from it here too. */
#define BOX_ROT(scale) ((SVECTOR *)(scale) - 1)
#define BOX_POS(scale) ((VECTOR *)((SVECTOR *)(scale) - 3))

void BtlTextWindowDraw(void)
{
    SPRT_16        glyph;
    DR_MODE        mode;
    DR_AREA        area;
    RECT           clip;
    MATRIX         m;
    SVECTOR        corner[4];
    POLY_FT4       quad;
    long           ofx;
    long           ofy;
    long           otz;
    BtlWindowCell *cell;
    VECTOR        *scale;
    int            i;

    cell = g_btl_text.cells;
    if (g_btl_text.placed == 0) {
        return;
    }
    scale = &g_btl_box_scale;
    if (scale->vx < BTL_BOX_FULL || g_btl_box_scale.vy < BTL_BOX_FULL) {
        SetPolyFT4(&quad);
        SetSemiTrans(&quad, 0);
        SetShadeTex(&quad, 1);
        ReadGeomOffset(&ofx, &ofy);
        SetGeomOffset(g_btl_text.x, g_btl_text.y);
        RotMatrix(BOX_ROT(scale), &m);
        TransMatrix(&m, BOX_POS(scale));
        ScaleMatrix(&m, scale);
        SetRotMatrix(&m);
        SetTransMatrix(&m);
        for (i = 0; i < g_btl_text.placed; cell++, i++) {
            for (otz = 0; otz < 4; otz++) {
                corner[otz].vx = cell->x + (otz & 1) * TEXT_GLYPH
                                 + g_btl_text.dx;
                corner[otz].vy = cell->y + (otz / 2) * TEXT_GLYPH
                                 - TEXT_RAISE + g_btl_text.dy;
                corner[otz].vz = 0;
            }
            RotTransPers4(&corner[0], &corner[1], &corner[2], &corner[3],
                          (long *)&quad.x0, (long *)&quad.x1,
                          (long *)&quad.x2, (long *)&quad.x3, &otz, &otz);
            quad.u0 = (i % BTL_GLYPH_ROW) * TEXT_GLYPH;
            quad.v0 = (i / BTL_GLYPH_ROW) * TEXT_GLYPH + TEXT_GLYPH_V;
            quad.u1 = quad.u0 + TEXT_GLYPH;
            quad.v1 = quad.v0;
            quad.u2 = quad.u0;
            quad.v2 = quad.v0 + TEXT_GLYPH;
            quad.u3 = quad.u0 + TEXT_GLYPH;
            quad.v3 = quad.v0 + TEXT_GLYPH;
            quad.tpage = GetTPage(0, 0,
                                  (g_btl_text_page * BTL_TEXT_PAGE_W
                                   + BTL_TEXT_PAGE0) * BTL_TEXT_COL,
                                  BTL_TEXT_GLYPH_Y);
            quad.clut = GetClut((g_btl_text_page * BTL_TEXT_PAGE_W
                                 + BTL_TEXT_PAGE0) * BTL_TEXT_COL,
                                cell->attr + BTL_TEXT_CLUT_Y);
            memcpy(g_btl_prim_next, &quad, sizeof(POLY_FT4));
            AddPrim(g_btl_ot[g_btl_ot_index], g_btl_prim_next);
            g_btl_prim_next += sizeof(POLY_FT4);
        }
        return;
    }

    clip.w = BTL_AREA_W;
    clip.h = BTL_AREA_H;
    clip.x = g_btl_draw_x;
    clip.y = g_btl_draw_y;
    SetDrawArea(&area, &clip);
    memcpy(g_btl_prim_next, &area, sizeof(DR_AREA));
    AddPrim(g_btl_ot[g_btl_ot_index], g_btl_prim_next);
    g_btl_prim_next += sizeof(DR_AREA);
    SetDrawMode(&mode, 0, 0,
                GetTPage(0, 0,
                         (g_btl_text_page * BTL_TEXT_PAGE_W + BTL_TEXT_PAGE0)
                             * BTL_TEXT_COL,
                         BTL_TEXT_GLYPH_Y),
                0);
    for (i = 0; i < g_btl_text.placed; cell++, i++) {
        SetSprt16(&glyph);
        SetSemiTrans(&glyph, 0);
        SetShadeTex(&glyph, 1);
        glyph.x0 = g_btl_text.dx + g_btl_text.x + cell->x;
        glyph.y0 = g_btl_text.dy + g_btl_text.y + cell->y
                   - TEXT_RAISE + g_btl_text.slide;
        glyph.u0 = (i % BTL_GLYPH_ROW) * TEXT_GLYPH;
        glyph.v0 = (i / BTL_GLYPH_ROW) * TEXT_GLYPH + TEXT_GLYPH_V;
        glyph.clut = GetClut((g_btl_text_page * BTL_TEXT_PAGE_W
                              + BTL_TEXT_PAGE0) * BTL_TEXT_COL,
                             cell->attr + BTL_TEXT_CLUT_Y);
        memcpy(g_btl_prim_next, &glyph, sizeof(SPRT_16));
        AddPrim(g_btl_ot[g_btl_ot_index], g_btl_prim_next);
        g_btl_prim_next += sizeof(SPRT_16);
    }
    memcpy(g_btl_prim_next, &mode, sizeof(DR_MODE));
    AddPrim(g_btl_ot[g_btl_ot_index], g_btl_prim_next);
    clip.w = BTL_AREA_W;
    clip.h = TEXT_BAND_H;
    g_btl_prim_next += sizeof(DR_MODE);
    clip.x = g_btl_draw_x;
    clip.y = g_btl_draw_y + g_btl_text.y + g_btl_text.dy - TEXT_RAISE;
    SetDrawArea(&area, &clip);
    memcpy(g_btl_prim_next, &area, sizeof(DR_AREA));
    AddPrim(g_btl_ot[g_btl_ot_index], g_btl_prim_next);
    g_btl_prim_next += sizeof(DR_AREA);
}
