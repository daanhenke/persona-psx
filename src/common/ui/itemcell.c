/* Persona 1 (JP) - one cell of the pending-item grid.
 *   ADV 0x8008EC64   S2D 0x80083218
 *
 * The same row itemrow.c draws, taken out of the two-column grid rather than
 * the flat list, and out of whichever glyph bank the caller asks for. A unit
 * of its own: ADV puts it ahead of the flat form and S2D behind it.
 */
#include <decomp/types.h>
#include <persona/common/item.h>

/* The staging list, reached by hardcoded address like the rest of the overlay
   work area. S2D's sits 0x20000 higher, which is what WORK_BIAS carries. */

#define ITEM_SHIFT 9

/* The row: twelve cells wide in a layer of forty, with the count eleven cells
   in. */
#define ROW_CELLS  0xC
#define ROW_STRIDE 0x28
#define ROW_COUNT_AT 11

/* Two digits, drawn from the font's zero. */
#define COUNT_DIGITS 2
#define GLYPH_DIGIT0 0xC0

/* The grid is two cells wide, and each glyph bank is this many glyphs on from
   the last. */
#define GRID_COLS  2
#define GLYPH_BANK 215

extern u_char g_hud_digits[];

extern short FormatDecimal(u_int value, u_char *dst, u_short width);
extern void  TileMapWriteRowRev(const u_char *src, short *dst, u_short base,
                                u_short count);
extern void  TileMapFillRect(short *dst, short value, u_short w, u_short h,
                             u_short stride);
extern void  DrawItemName(int id, short *dst, u_short base, int b);

/* The same cell drawn out of the two-column grid rather than the flat list,
   and out of whichever glyph bank the caller asks for. */
void DrawItemCell(short *dst, short col, short row, u_char bank)
{
    u_short (*grid)[GRID_COLS];
    u_short  *entry;
    int       base;
    short     n;

    grid = (u_short (*)[GRID_COLS])g_items_pending;
    TileMapFillRect(dst, 0, ROW_CELLS, 1, ROW_STRIDE);
    /* One scratch: the id test first, then the glyph bank. Do not split it. */
    base = (grid[row][col] & ITEM_ID) != 0;
    entry = &grid[row][col];
    if (base && (*entry >> ITEM_SHIFT) != 0) {
        base = bank * GLYPH_BANK;
        DrawItemName(grid[row][col] & ITEM_ID, dst, base, 0);
        n = FormatDecimal(*entry >> ITEM_SHIFT, g_hud_digits, COUNT_DIGITS);
        TileMapWriteRowRev(g_hud_digits, &dst[ROW_COUNT_AT],
                           base + GLYPH_DIGIT0, n);
    }
}
