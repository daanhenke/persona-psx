/* Persona 1 (JP) - the board the fight picks an item off.  BTLP only.
 *   0x800AB2E0 BtlCloseStockBoard  0x800AB308 BtlOpenItemBoard
 *   0x800AB350 BtlCloseItemBoard   0x800AB378 BtlBuildItemLines
 *
 * Twelve lines, filled from the inventory before the board goes up.
 *
 * BtlBuildItemLines does it in two passes. The first walks the inventory from
 * wherever the menu has got to and copies the entries worth offering into a
 * list of twelve, stopping at the end of the inventory or when the list is
 * full, and clears whatever is left of the list; the second turns each of
 * those entries into a line - the item's name, its count, and the height of
 * the two rows the line is drawn as.
 *
 * Only ten lines are ever seen: the last two are given a height of zero, so
 * the list holds twelve and the board shows ten. A slot the list did not fill
 * ends up with id zero, whose record is the placeholder, and its count writes
 * the end marker rather than a number.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <decomp/libc.h>
#include <persona/btlp/board.h>
#include <persona/btlp/number.h>
#include <persona/btlp/object.h>
#include <persona/common/item.h>

/* Lines the list holds, and how many of them are drawn. */
#define ITEM_LINES 12
#define ITEM_ROWS  10

/* How tall a line that is drawn stands. */
#define ITEM_LINE_H 12

/* Ids at or above this are equipment rather than something to use in a
   fight, and the board skips them. */
#define ITEM_CONSUMABLE_MAX 0x56

/* Characters a count is written in. */
#define ITEM_COUNT_WIDTH 2

extern const BtlBoardDef g_btl_list_board_defs[];
extern const long        g_btl_item_board_pos[];
extern BtlObj           *g_btl_list_board;
extern BtlObj           *g_btl_stock_board;

/* Where the menu has got to in the inventory. */
extern u_short *g_btl_item_at;

/* The twelve entries the board is drawn from, and the three things each line
   is made of: the name, the count, and the two rows it is drawn as. */
extern u_short   g_btl_item_slots[];
extern u_char    g_btl_item_names[][10];
extern u_char    g_btl_item_counts[][3];
extern BtlGfxText g_btl_item_rows[];
extern BtlGfxText g_btl_item_rows2[];

void BtlBuildItemLines(u_short *from);

void BtlCloseStockBoard(void)
{
    BtlBoardShut(g_btl_stock_board);
}

void BtlOpenItemBoard(void)
{
    BtlBuildItemLines(g_btl_item_at);
    g_btl_list_board = BtlBoardOpen(g_btl_list_board_defs, g_btl_item_board_pos);
}

void BtlCloseItemBoard(void)
{
    BtlBoardShut(g_btl_list_board);
}

#ifdef NON_MATCHING
void BtlBuildItemLines(u_short *from)
{
    BtlGfxText  *rows;
    BtlGfxText  *rows2;
    u_char      *name;
    u_char      *count_at;
    u_short     *slot;
    u_short      entry;
    signed char *h;
    signed char *h2;
    signed char  tall;
    int          id;
    int          count;
    int          n;

    rows     = g_btl_item_rows;
    rows2    = rows + ITEM_LINES;
    name     = g_btl_item_names[0];
    count_at = g_btl_item_counts[0];
    n        = 0;
    while (n < ITEM_LINES) {
        if (from >= &g_items[ITEM_SLOTS]) {
            break;
        }
        entry = *from;
        id    = entry & ITEM_ID;
        count = *from >> ITEM_SHIFT;
        if (id != 0 && id < ITEM_CONSUMABLE_MAX) {
            if (count != 0 && (g_item_defs[id].unk06 & USABLE_MARK) != 0) {
                g_btl_item_slots[n] = entry;
                n++;
            }
        }
        from++;
    }

    if (n < ITEM_LINES) {
        slot = &g_btl_item_slots[n];
        do {
            *slot = 0;
            n++;
            slot++;
        } while (n < ITEM_LINES);
    }

    n    = 0;
    tall = ITEM_LINE_H;
    h2   = &rows2->h;
    h    = &rows->h;
    slot = g_btl_item_slots;
    do {
        memcpy(name, g_item_defs[*slot & ITEM_ID].name,
               sizeof(g_btl_item_names[0]));
        count = *slot >> ITEM_SHIFT;
        if (count == 0) {
            *count_at = BTL_TEXT_END;
        } else {
            BtlDrawNumber(count_at, count, ITEM_COUNT_WIDTH);
        }
        if (n < ITEM_ROWS) {
            *h = tall;
        } else {
            *h = 0;
        }
        if (n < ITEM_ROWS) {
            *h2 = tall;
        } else {
            *h2 = 0;
        }
        slot++;
        n++;
        name     += sizeof(g_btl_item_names[0]);
        count_at += sizeof(g_btl_item_counts[0]);
        h        += sizeof(BtlGfxText);
        h2       += sizeof(BtlGfxText);
    } while (n < ITEM_LINES);
}
#else
INCLUDE_ASM("btlp/nonmatchings/itemboard", BtlBuildItemLines);
#endif
