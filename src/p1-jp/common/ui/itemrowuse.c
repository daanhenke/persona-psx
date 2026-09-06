/* Persona 1 (JP) - one row of the item list, greyed when it would do nothing.
 *
 * Compiled into two overlays rather than called across the boundary:
 *   ADV 0x8006AC50   S2D 0x8006A1DC
 *
 * The same row as DrawItemRow - name at the left, count right-aligned two
 * digits along - but drawn out of one of two glyph banks. An entry marked as
 * worth asking about gets the plain bank when it would still do something and
 * the greyed one when it would not; everything else is greyed, so the page
 * only shows a plain row for a recovery item somebody needs.
 *
 * One local carries the item id in and the chosen bank out; do not split it.
 */
#include <types.h>
#include <persona/common/item.h>

/* The staging list, reached by hardcoded address; S2D's sits 0x20000
   higher. */
#define g_items_pending ((u_short *)(0x800EAE4C + WORK_BIAS))

#define ITEM_ID    0x1FF
#define ITEM_SHIFT 9

/* The bit that says the entry's usefulness is worth asking about. */
#define ITEM_ASKABLE 0x40

/* The row: twelve cells wide in a layer of forty, with the count eleven cells
   in. */
#define ROW_CELLS    0xC
#define ROW_STRIDE   0x28
#define ROW_COUNT_AT 11

/* Two digits, drawn from the font's zero. */
#define COUNT_DIGITS 2
#define GLYPH_DIGIT0 0xC0

/* Each glyph bank is this many glyphs on from the last. */
#define GLYPH_BANK 215

extern u_char g_hud_digits[];

extern u_char SpellUsable(short spell);
extern short  FormatDecimal(u_int value, u_char *dst, u_short width);
extern void   TileMapWriteRowRev(const u_char *src, short *dst, u_short base,
                                 u_short count);
extern void   TileMapFillRect(short *dst, short value, u_short w, u_short h,
                              u_short stride);
extern void   DrawItemName(int id, short *dst, u_short base, int b);

void DrawItemRowUsable(short slot, short *dst)
{
    u_short *entry;
    u_short *count;
    int      bank;
    int      base;
    short    n;

    TileMapFillRect(dst, 0, ROW_CELLS, 1, ROW_STRIDE);
    count = &g_items_pending[slot];
    entry = &g_items_pending[slot];
    bank = *entry & ITEM_ID;
    if ((g_items_pending[slot] & ITEM_ID) != 0 &&
        (*count >> ITEM_SHIFT) != 0) {
        if ((g_item_defs[g_items_pending[slot] & ITEM_ID].unk06 &
             ITEM_ASKABLE) != 0 && SpellUsable(bank) != 0) {
            bank = 0;
        } else {
            bank = 1;
        }
        base  = bank * GLYPH_BANK;
        entry = &g_items_pending[slot];
        DrawItemName(*entry & ITEM_ID, dst, base, 0);
        count = entry;
        n = FormatDecimal(*count >> ITEM_SHIFT, g_hud_digits, COUNT_DIGITS);
        TileMapWriteRowRev(g_hud_digits, &dst[ROW_COUNT_AT],
                           base + GLYPH_DIGIT0, n);
    }
}
