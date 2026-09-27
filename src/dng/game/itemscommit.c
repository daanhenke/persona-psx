/* Persona 1 (JP) - writing the staged item list back to the inventory.
 * DNG only.
 *   0x800929D8 ItemsCommitPending
 *
 * Every entry left in g_items_pending goes back into g_items: over the
 * slot already holding the same item, or into the first slot that is not in
 * use (id and count both non-zero count as in use).
 */
#include <decomp/types.h>
#include <persona/common/item.h>

void ItemsCommitPending(void)
{
    u_long   pend;   /* added index-first: an integer, not a pointer */
    u_short *items;
    u_short *p;
    short    i;
    short    j;

    pend = (u_long)g_items_pending;
    items = g_items;
    for (i = 0; i < ITEM_SLOTS; i++) {
        p = (u_short *)(i * 2 + pend);
        if (*p != 0) {
            j = ItemsFind(*p & ITEM_ID);
            if (j >= 0) {
                items[j] = *p;
            } else {
                u_short *src = p;

                do {
                    j++;
                } while ((items[j] & ITEM_ID) && (items[j] >> 9));
                items[j] = *src;
            }
        }
    }
}
