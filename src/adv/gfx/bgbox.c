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
#include <decomp/types.h>
#include <persona/adv/moneybox.h>
#include <persona/common/bg.h>
#include <persona/common/item.h>
#include <persona/common/tilemap.h>

/* Which layer the box is, and where it sits. */
#define BOX_LAYER 5
#define BOX_BIT   0x20
#define BOX_X     0xD8
#define BOX_Y     0x98
#define BOX_W     0x60
#define BOX_H     0x24
#define BOX_OTZ   0x3E

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
    n = FormatDecimal(G_MONEY, g_hud_digits, MONEY_DIGITS);
    TileMapWriteRowRev(g_hud_digits, &cells[MONEY_AT], GLYPH_DIGIT0, n);
}
