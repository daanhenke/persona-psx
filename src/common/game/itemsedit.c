/* Persona 1 (JP) - adding to and taking from the held list.  ADV only.
 *   0x800B08F4 ItemsAdd   0x800B09E0 ItemsRemove
 *
 * The persistent-list versions of the two staging routines, and the only part
 * of the item source that is ADV's alone - it sits far away from the rest,
 * which is in items.c and itemslist.c.
 *
 * ItemsCompact runs either side of the edit, which is what stops a half-filled
 * slot surviving: a subtraction that empties an entry leaves the id behind,
 * and the compaction is what zeroes it.
 *
 * The lists themselves and the packing are in persona/common/item.h.
 */
#include <decomp/types.h>
#include <persona/common/item.h>

/* Both are defined above these in the original, so the calls below have their
   prototypes in scope; the definitions now live in other units. */
extern short ItemsFind(u_short id);
extern void  ItemsCompact(void);

/* The base is spelled out again inside the search loop on purpose - do not
   tidy it away. */
void ItemsAdd(u_short id, short count)
{
    u_short *list;
    u_short *slot;
    short    i;
    short    n;
    int      packed;

    list = g_items;
    ItemsCompact();
    i = ItemsFind(id);
    if (i == -1) {
        for (;;) {
            short j;

            i++;
            list = g_items;
            j = i;
            slot = &list[j];
            if ((*slot >> 9) == 0) {
                break;
            }
            if ((*slot & ITEM_ID) == 0) {
                break;
            }
        }
        *slot = 0;
    }
    n = count + (list[i] >> 9);
    packed = id & ITEM_ID;
    if (n > ITEM_MAX) {
        n = ITEM_MAX;
    }
    list[i] = packed + n * 0x200;
    ItemsCompact();
}

/* Nothing checks the item is held: ItemsFind answering -1 indexes one entry
   before the list. */
void ItemsRemove(u_short id, short count)
{
    u_short *list;
    u_short *slot;
    short    n;

    list = g_items;
    ItemsCompact();
    slot = &list[ItemsFind(id)];
    n = (*slot >> 9) - count;
    *slot = (id & ITEM_ID) + n * 0x200;
    ItemsCompact();
}
