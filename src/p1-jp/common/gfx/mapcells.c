/* Persona 1 (JP) - the cell definitions behind the character maps.
 *
 * Compiled into three overlays rather than called across the boundary:
 *                  DNG         ADV         S2D
 *   FontCellsInit  0x8008B36C  0x8007CDD4  0x8007B7D4
 *
 * A GsMAP draws through two arrays: an index array saying which cell sits in
 * each map square, and a cell array saying what each cell shows. This fills
 * the cell array the text map points at - the screen setup at 0x80068A7C
 * stores g_font_cells straight into that map's `base`.
 *
 * The glyphs are an 8x12 atlas 31 columns wide starting at v 0x50, so cell
 * n + 1 is glyph n and cell 0 is the blank at the far corner of the page. That
 * offset by one is why an index array cleared to zero draws as empty space.
 *
 * S2D keeps its own copy against a work area 0x20000 higher; see
 * src/p1-jp/s2d/gfx/mapcells.c.
 */
#include <types.h>
#include <libgs.h>

/* Reached by hardcoded address; the blank is the cell before the atlas. */
#define g_font_blank ((GsCELL *)0x800E864C)
#define g_font_cells ((GsCELL *)0x800E8654)

#define GLYPH_W    8
#define GLYPH_H    12
#define GLYPH_COLS 31
#define GLYPH_ROWS 4
#define FONT_V     0x50
#define FONT_CLUT  0x7E80
#define FONT_TPAGE 0x18

void FontCellsInit(void)
{
    short i;

    g_font_blank->u = 0xF8;
    g_font_blank->v = 0xF4;
    g_font_blank->cba = FONT_CLUT;
    g_font_blank->flag = 0;
    g_font_blank->tpage = FONT_TPAGE;
    for (i = 0; i < GLYPH_COLS * GLYPH_ROWS; i++) {
        g_font_cells[i].u = (i % GLYPH_COLS) * GLYPH_W;
        g_font_cells[i].v = (i / GLYPH_COLS) * GLYPH_H + FONT_V;
        g_font_cells[i].cba = FONT_CLUT;
        g_font_cells[i].flag = 0;
        g_font_cells[i].tpage = FONT_TPAGE;
    }
}
