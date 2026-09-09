/* Persona 1 (JP) - clearing the staging list and tidying the held one.
 *
 * Compiled into three overlays rather than called across the boundary:
 *              DNG         ADV         S2D
 *   Compact    0x80092B30  0x8008EA90  0x80083044
 *
 * The tail of the item unit: the commit routine before these is not worked out
 * yet, and a short copy helper that belongs to another source sits between it
 * and these two, so the overlays take both stretches from asm. The head of the
 * unit is in items.c.
 *
 * The lists themselves and the packing are in persona/common/item.h.
 */
#include <decomp/types.h>
#include <persona/common/item.h>

/* Empties the staging list. The item menus refill it with the subset of
   g_items they are about to page through, so it doubles as the working list
   for a menu as well as the pending-merge buffer. */
void ItemsClearPending(void)
{
    u_short *p;
    int      i;

    i = ITEM_SLOTS - 1;
    p = &g_items_pending[ITEM_SLOTS - 1];
    for (; i >= 0; i--) {
        *p-- = 0;
    }
}

/* Zeroes any half-filled slot in the persistent list - an entry with a count
   but no id, or an id but no count - so the whole word reads as free. */
void ItemsCompact(void)
{
    u_short *p;
    int      i;

    for (i = 0, p = g_items; i < ITEM_SLOTS; i++) {
        if ((*p >> 9) == 0 || (*p & ITEM_ID) == 0) {
            *p = 0;
        }
        p++;
    }
}
