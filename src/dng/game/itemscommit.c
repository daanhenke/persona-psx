/* Persona 1 (JP) - writing the staged item list back to the inventory.
 * DNG's source; ADV builds it too.
 *   0x800929D8 ItemsCommitPending
 *
 * Every entry left in g_items_pending goes back into g_items: over the
 * slot already holding the same item, or into the first slot that is not in
 * use (id and count both non-zero count as in use).
 */
#include <decomp/types.h>
#include <persona/common/item.h>

/* The staging entry, indexed afresh at each use: ADV's build, with the
   search inline, reads it again after the search rather than holding it. */
#define PENDING(i) ((u_short *)((i) * 2 + pend))

void ItemsCommitPending(void)
{
    u_long   pend;   /* added index-first: an integer, not a pointer */
    u_short *items;
    short    i;
    short    j;

    pend = (u_long)g_items_pending;
    items = g_items;
    for (i = 0; i < ITEM_SLOTS; i++) {
        if (*PENDING(i) != 0) {
            j = ItemsFind(*PENDING(i) & ITEM_ID);
            if (j >= 0) {
                items[j] = *PENDING(i);
            } else {
                u_short *src = PENDING(i);

                do {
                    j++;
                } while ((items[j] & ITEM_ID) && (items[j] >> 9));
                items[j] = *src;
            }
        }
    }
}
