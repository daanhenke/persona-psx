/* Persona 1 (JP) - the item grid's cells, and the menu's background layers.
 *   DNG 0x80092C78 DrawItemCellRows   0x80092D04 DrawItemCell
 *       0x80092E10 BgLineShow         0x80092E44 BgLineHide
 *       0x80092E5C MenuSetLayers
 *
 * DrawItemCell is the field's build of the shared cell (src/common/ui/
 * itemcell.c): defined old-style, with the id read into a local of its own.
 * MenuSetLayers puts each menu page's four background layers where the page
 * wants them, from a table of one record per page.
 */
#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <persona/common/bg.h>

/* The list the grid is drawn from: two columns of entries, an item id in the
   low nine bits and a count above them. */
#define g_item_list ((u_short *)0x800EAE4C)
#define ITEM_ID     0x1FF
#define ITEM_SHIFT  9
#define GRID_COLS   2

/* A cell: twelve cells wide in a layer of forty, the count eleven in, two
   digits from the font's zero; each glyph bank is 0xD7 on from the last. */
#define MAP_W        40
#define ROW_CELLS    0xC
#define ROW_COUNT_AT 11
#define GLYPH_DIGIT0 0xC0
#define GLYPH_BANK   0xD7

/* The one-line layer the item and message lines are drawn on. */
#define LINE_LAYER 4

/* A menu page's layers: which are shown, then each layer's rectangle. */
typedef struct {
    /* 0x00 */ u_char shown;
    /* 0x02 */ short  rect[4][4];   /* x, y, w, h */
} MenuLayers;                       /* 0x22 bytes */

extern MenuLayers g_menu_layers[];
extern u_char     g_hud_digits[];
extern short      g_map_scroll_x;
extern short      g_map_scroll_y;
extern short      g_header_scroll_x;
extern short      g_header_scroll_y;

extern short FormatDecimal(u_int value, u_char *dst, u_short width);
extern void  TileMapWriteRowRev(const u_char *src, short *dst, int base,
                                int count);
extern void  TileMapFillRect(short *dst, short value, u_short w, u_short h,
                             u_short stride);
extern void  DrawItemName(int id, short *dst, u_short base, int b);

void DrawItemCell();

/* `count` rows of the grid from `row` on, both columns. */
void DrawItemCellRows(dst, row, count)
    short *dst;
    u_char row;
    u_char count;
{
    while (count != 0) {
        DrawItemCell(dst, 0, (short)row, 0);
        DrawItemCell(dst + 14, 1, (short)row, 0);
        dst += MAP_W;
        count--;
        row++;
    }
}

/* One cell of the grid, out of the glyph bank the caller asks for. */
void DrawItemCell(dst, col, row, bank)
    short *dst;
    short  col;
    short  row;
    u_char bank;
{
    u_short (*grid)[GRID_COLS] = (u_short (*)[GRID_COLS])g_item_list;
    u_short *entry;
    int      id;
    int      base;
    short    n;

    TileMapFillRect(dst, 0, ROW_CELLS, 1, MAP_W);
    id = grid[row][col] & ITEM_ID;
    entry = &grid[row][col];
    if (id != 0 && (*entry >> ITEM_SHIFT) != 0) {
        base = bank * GLYPH_BANK;
        DrawItemName(id, dst, base, 0);
        n = FormatDecimal(*entry >> ITEM_SHIFT, g_hud_digits, 2);
        TileMapWriteRowRev(g_hud_digits, &dst[ROW_COUNT_AT],
                           base + GLYPH_DIGIT0, n);
    }
}

void BgLineShow(void)
{
    g_bg_layers[LINE_LAYER].x = 0x10;
    g_bg_layers[LINE_LAYER].y = 8;
    g_bg_layers[LINE_LAYER].w = 0xF0;
    g_bg_layers[LINE_LAYER].h = 0x10;
}

void BgLineHide(void)
{
    g_bg_layers[LINE_LAYER].w = 0;
    g_bg_layers[LINE_LAYER].h = 0;
}

void MenuSetLayers(page)
    short page;
{
    MenuLayers *l = &g_menu_layers[page];

    g_bg_shown = l->shown;
    g_bg_layers[0].x = l->rect[0][0];
    g_bg_layers[0].y = l->rect[0][1];
    g_bg_layers[0].w = l->rect[0][2];
    g_bg_layers[0].h = l->rect[0][3];
    g_bg_layers[0].scrollx = 0;
    g_bg_layers[0].scrolly = 0;
    g_bg_layers[1].x = l->rect[1][0];
    g_bg_layers[1].y = l->rect[1][1];
    g_bg_layers[1].w = l->rect[1][2];
    g_bg_layers[1].h = l->rect[1][3];
    g_bg_layers[1].scrollx = 0;
    g_bg_layers[1].scrolly = 0;
    g_bg_layers[2].x = l->rect[2][0];
    g_bg_layers[2].y = l->rect[2][1];
    g_bg_layers[2].w = l->rect[2][2];
    g_bg_layers[2].h = l->rect[2][3];
    g_bg_layers[2].scrollx = 0;
    g_bg_layers[2].scrolly = 0;
    g_bg_layers[3].x = l->rect[3][0];
    g_bg_layers[3].y = l->rect[3][1];
    g_bg_layers[3].w = l->rect[3][2];
    g_bg_layers[3].h = l->rect[3][3];
    g_map_scroll_x = 0;
    g_map_scroll_y = 0;
    g_header_scroll_x = 0;
    g_header_scroll_y = 0;
    g_bg_layers[3].scrollx = 4;
    g_bg_layers[3].scrolly = 4;
}
