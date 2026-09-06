/* Persona 1 (JP) - the money box.  ADV @ 0x8007CFC8.
 *
 * The same shape as BgPanelSet one layer along: a small background layer sized
 * and placed, given a depth, and switched on. What goes in it is the party's
 * money, right-aligned in nine digits with two labels above and beside it.
 *
 * The labels are runs of consecutive glyphs rather than text of their own -
 * str_cell_run counts 0, 1, 2, 3 and TileMapWriteRow adds the base, so one
 * ascending run draws any stretch of the font.
 */
#include <types.h>
#include <libgs.h>

/* Which layer the box is, and where it sits. */
#define BOX_LAYER 5
#define BOX_BIT   0x20
#define BOX_X     0xD8
#define BOX_Y     0x98
#define BOX_W     0x60
#define BOX_H     0x24
#define BOX_OTZ   0x3E

/* The cell run behind it: twelve across, four down. */
#define BOX_CELLS_W 0xC
#define BOX_CELLS_H 4

/* The two labels, and the cell that closes the amount off. */
#define LABEL_TOP_AT    0
#define LABEL_TOP_BASE  0x368
#define LABEL_TOP_LEN   5
#define LABEL_SIDE_AT   24
#define LABEL_SIDE_BASE 0x364
#define LABEL_SIDE_LEN  4
#define UNIT_AT         13
#define UNIT_GLYPH      0x363

/* Nine digits, drawn backwards from the font's zero. */
#define MONEY_DIGITS 9
#define MONEY_AT     22
#define GLYPH_DIGIT0 0xC0

extern GsBG         g_bg_layers[];
extern u_short      g_bg_layer_otz[];
extern u_long       g_bg_shown;
extern short        g_panel_cells[];
extern u_char       g_hud_digits[];
extern u_int        g_money;
extern const u_char str_cell_run[];

extern short FormatDecimal(u_int value, u_char *dst, u_short width);
extern void  TileMapWriteRow(const u_char *src, short *dst, int base,
                             u_short count);
extern void  TileMapWriteRowRev(const u_char *src, short *dst, int base,
                                u_short count);
extern void  TileMapFillRect(short *dst, short value, u_short w, u_short h,
                             u_short stride);

void BgBoxShow(void)
{
    short *cells;
    short  n;

    cells = g_panel_cells;
    g_bg_layers[BOX_LAYER].x = BOX_X;
    g_bg_layers[BOX_LAYER].y = BOX_Y;
    g_bg_layers[BOX_LAYER].w = BOX_W;
    g_bg_layers[BOX_LAYER].h = BOX_H;
    g_bg_layer_otz[BOX_LAYER] = BOX_OTZ;
    g_bg_shown |= BOX_BIT;
    TileMapFillRect(cells, 0, BOX_CELLS_W, BOX_CELLS_H, BOX_CELLS_W);

    TileMapWriteRow(str_cell_run, &cells[LABEL_TOP_AT], LABEL_TOP_BASE,
                    LABEL_TOP_LEN);
    TileMapWriteRow(str_cell_run, &cells[LABEL_SIDE_AT], LABEL_SIDE_BASE,
                    LABEL_SIDE_LEN);
    g_panel_cells[UNIT_AT] = UNIT_GLYPH;
    n = FormatDecimal(g_money, g_hud_digits, MONEY_DIGITS);
    TileMapWriteRowRev(g_hud_digits, &cells[MONEY_AT], GLYPH_DIGIT0, n);
}
