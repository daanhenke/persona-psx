/* Persona 1 (JP) - one row of the pending-item list.
 *
 *   ADV @ 0x80093608   S2D @ 0x8007E3EC
 *
 * An entry packs both halves into one u16 - the low nine bits are the item id
 * and the top seven the count - so a slot counts as holding something only
 * when neither half is zero. A slot that fails either test leaves the row
 * cleared and nothing else is drawn.
 *
 * The name goes at the left and the count right-aligned two digits along,
 * written backwards from the font's zero so the units digit lands in a fixed
 * column.
 */
#include <types.h>

/* The staging list, reached by hardcoded address like the rest of the overlay
   work area. S2D's sits 0x20000 higher, which is what WORK_BIAS carries. */
#define g_items_pending ((u_short *)(0x800EAE4C + WORK_BIAS))

#define ITEM_ID    0x1FF
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

void DrawItemRow(short slot, short *dst)
{
    u_short *entry;
    short    n;

    TileMapFillRect(dst, 0, ROW_CELLS, 1, ROW_STRIDE);
    entry = &g_items_pending[slot];
    if ((*entry & ITEM_ID) != 0 && (*entry >> ITEM_SHIFT) != 0) {
        DrawItemName(*entry & ITEM_ID, dst, 0, 0);
        n = FormatDecimal(*entry >> ITEM_SHIFT, g_hud_digits, COUNT_DIGITS);
        TileMapWriteRowRev(g_hud_digits, &dst[ROW_COUNT_AT], GLYPH_DIGIT0, n);
    }
}

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
    entry = &grid[row][col];
    if ((*entry & ITEM_ID) != 0 && (*entry >> ITEM_SHIFT) != 0) {
        base = bank * GLYPH_BANK;
        DrawItemName(*entry & ITEM_ID, dst, base, 0);
        n = FormatDecimal(*entry >> ITEM_SHIFT, g_hud_digits, COUNT_DIGITS);
        TileMapWriteRowRev(g_hud_digits, &dst[ROW_COUNT_AT],
                           base + GLYPH_DIGIT0, n);
    }
}
