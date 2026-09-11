/* Persona 1 (JP) - the items a fight can offer.  BTLP only.
 *   0x800AB58C BtlNextUsableItem  0x800AB604 BtlPrevUsableItem
 *   0x800AB694 BtlFirstUsableItem
 *   0x800AB700 BtlBuildUsableItems 0x800AB76C BtlCommitUsableItems
 *   0x800AB7D8 BtlOpenSpellBoard  0x800AB82C BtlCloseListBoard
 *
 * The battle stages the items worth offering into a list of its own rather
 * than editing the party's: BtlBuildUsableItems copies every inventory entry
 * whose record carries the mark, and BtlCommitUsableItems writes the counts
 * back. The commit walks the inventory once across the whole of the staged
 * list rather than searching from the start each time, which works because
 * both lists are in the same order.
 *
 * The three searches are for the menu that reads the inventory directly.
 * Each one steps a slot at a time and answers with the second entry it finds
 * that is both a consumable and carries the mark - the first is the one the
 * cursor is already on. The two that walk are given a slot to start from and
 * step either way; the one that does not starts at the head of the list and
 * answers the first entry it finds, not the second. The backward walk is the
 * only one that can fail, and it answers nothing when it reaches the head.
 *
 * Neither forward walk is bounded. Whatever the menu passes in is a slot it
 * has already found, so there is always an end marker in front of it - an
 * inventory of nothing but usable items would run off the end of the array.
 *
 * BtlOpenSpellBoard is the same board BtlOpenItemBoard uses, filled from the
 * member's spell list instead of the inventory; only one of the two is ever
 * up, which is why the close belongs to neither.
 */
#include <decomp/types.h>
#include <persona/btlp/board.h>
#include <persona/btlp/object.h>
#include <persona/common/item.h>

/* Ids at or above this are equipment rather than something to use in a
   fight, and the menu skips them. */
#define ITEM_CONSUMABLE_MAX 0x56

/* Which entry a walk answers with. */
#define ITEM_WANTED 2

extern const BtlBoardDef g_btl_list_board_defs[];
extern const long        g_btl_spell_board_pos[];
extern BtlObj           *g_btl_list_board;
extern u_char            g_btl_list_open;
extern int               g_btl_spell_slot;

/* Fills the board's twelve lines from a member's spells. Still asm. */
extern void func_800AB85C(int slot);

/* Whether the entry in this slot is one the fight can offer. */
#define BtlItemOffered(id, count)                           \
    ((id) < ITEM_CONSUMABLE_MAX && (count) != 0              \
     && (g_item_defs[id].unk06 & USABLE_MARK) != 0)

u_short *BtlNextUsableItem(u_short *slot)
{
    int id;
    int count;
    int found;

    found = 0;
    for (;;) {
        slot++;
        id    = *slot & ITEM_ID;
        count = *slot >> ITEM_SHIFT;
        if (id == 0) {
            continue;
        }
        if (BtlItemOffered(id, count)) {
            found++;
            if (found >= ITEM_WANTED) {
                return slot;
            }
        }
    }
}

u_short *BtlPrevUsableItem(u_short *slot)
{
    int id;
    int count;
    int found;

    found = 0;
    for (;;) {
        slot--;
        if (slot < g_items) {
            return 0;
        }
        id    = *slot & ITEM_ID;
        count = *slot >> ITEM_SHIFT;
        if (id == 0) {
            continue;
        }
        if (BtlItemOffered(id, count)) {
            found++;
            if (found >= ITEM_WANTED) {
                return slot;
            }
        }
    }
}

u_short *BtlFirstUsableItem(void)
{
    u_short *slot;
    int      id;
    int      count;

    slot = g_items;
    do {
        id    = *slot & ITEM_ID;
        count = *slot >> ITEM_SHIFT;
        if (id != 0 && BtlItemOffered(id, count)) {
            return slot;
        }
        slot++;
    } while (slot < &g_items[ITEM_SLOTS]);
    return 0;
}

void BtlBuildUsableItems(void)
{
    u_short *slot;
    u_short *out;
    u_short  entry;
    int      id;
    int      count;
    int      i;

    slot = g_items;
    out  = g_btl_usable_items;
    i    = 0;
    do {
        entry = *slot;
        id    = entry & ITEM_ID;
        count = *slot >> ITEM_SHIFT;
        if (id != 0 && count != 0
            && (g_item_defs[id].unk06 & USABLE_MARK) != 0) {
            *out = entry;
            out++;
        }
        i++;
        slot++;
    } while (i < ITEM_SEARCH);
}

void BtlCommitUsableItems(void)
{
    u_short *slot;
    u_short *staged;
    u_short  entry;
    int      id;
    int      count;
    int      i;

    slot   = g_items;
    staged = g_btl_usable_items;
    i      = 0;
    do {
        entry = *staged;
        id    = entry & ITEM_ID;
        count = *staged >> ITEM_SHIFT;
        if (*staged != 0) {
            while ((*slot & ITEM_ID) != id) {
                slot++;
            }
            if (count != 0) {
                *slot = *staged;
            } else {
                *slot = 0;
            }
        }
        i++;
        staged++;
    } while (i < USABLE_ITEM_SLOTS);
}

void BtlOpenSpellBoard(void)
{
    g_btl_list_open = 1;
    func_800AB85C(g_btl_spell_slot);
    g_btl_list_board = BtlBoardOpen(g_btl_list_board_defs, g_btl_spell_board_pos);
}

void BtlCloseListBoard(void)
{
    g_btl_list_open = 0;
    BtlBoardShut(g_btl_list_board);
}
