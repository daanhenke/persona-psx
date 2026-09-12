/* Persona 1 (JP) - the battle's own way in and out of the party's bag.
 * BTLP only.
 *   0x80094EF4 BtlGiveItem  0x80094FAC BtlTakeItem
 *
 * A second pair on top of BtlItemAdd and BtlItemRemove, and the overlay
 * carries both: these walk g_items themselves rather than asking BtlItemSlot
 * where the entry is, which is why they are here and not in dropitems.c.
 *
 * Giving looks for an entry already carrying the id and counts it up; a stack
 * that is already at ninety-nine is refused outright rather than spilling into
 * a second slot. Only when nothing is carrying the id at all does it walk the
 * list a second time for a slot with either half clear and open a stack of one
 * there.
 *
 * Taking counts the first stack it finds down by one and answers whether it
 * found one. It does not clear an emptied slot - the id stays with a count of
 * nothing, which still reads as free to the giving walk.
 */
#include <decomp/types.h>
#include <persona/common/item.h>

/* The taking walk runs while the pointer is still inside the list, so its
   bound is the last byte of the last entry rather than the entry past it and
   the test is <=. */
#define ITEM_BAG_LAST ((u_short *)((u_char *)&g_items[ITEM_SLOTS] - 1))

void BtlGiveItem(int id)
{
    u_short *p;
    u_short  entry;
    int      given;
    int      i;

    i = 0;
    given = 0;
    p = g_items;
    do {
        entry = *p;
        if ((entry & ITEM_ID) == id) {
            /* The count is read off the entry again rather than out of the
               copy the id came from, which is where the image puts the second
               load. */
            if ((u_int)(*p >> ITEM_SHIFT) >= ITEM_MAX) {
                return;
            }
            *p = entry + (1 << ITEM_SHIFT);
            given = 1;
            break;
        }
        i++;
        p++;
    } while (i < ITEM_SLOTS);
    if (given != 0) {
        return;
    }

    i = 0;
    id |= 1 << ITEM_SHIFT;
    p = g_items;
    do {
        if ((*p & ITEM_ID) == 0 || (*p >> ITEM_SHIFT) == 0) {
            *p = id;
            return;
        }
        i++;
        p++;
    } while (i < ITEM_SLOTS);
}

int BtlTakeItem(int id)
{
    u_short *p;
    int      count;

    p = g_items;
    do {
        count = *p >> ITEM_SHIFT;
        if (id == (*p & ITEM_ID) && count > 0) {
            count--;
            *p = id | count << ITEM_SHIFT;
            return 1;
        }
        p++;
    } while (p <= ITEM_BAG_LAST);
    return 0;
}
