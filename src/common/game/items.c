/* Persona 1 (JP) - the staging item list.
 *
 * Compiled into three overlays rather than called across the boundary:
 *              DNG         ADV         S2D
 *   Find       0x80092918  0x8008E848  0x80082E2C
 *   FindPend   0x80092978  0x8008E8A8  0x80082E8C
 *   AddPend    0x800927C8  0x8008E6F8  0x80082CDC
 *
 * The commit routine below is not worked out yet, so the overlays take it from
 * asm and the rest of the original source is in itemslist.c, with ADV's own
 * two persistent-list edits in itemsedit.c.
 *
 * The lists themselves and the packing are in persona/common/item.h.
 */
#include <decomp/types.h>
#include <persona/common/item.h>

/* Declared before the two that call it, as the original has it: the prototype
   is what decides how the argument is converted, and the definition now sits
   below them rather than above. */
extern short ItemsFindPending(u_short id);

/* Tidies the staging list in three passes: fold entries that share an id into
   the earlier slot (counts still stop at 99), drop anything with an empty half,
   then slide the survivors down so the used entries are contiguous. */
void ItemsMergePending(void)
{
    u_short *list;
    u_short  i;
    u_short  j;
    u_int    count;

    list = g_items_pending;

    for (i = 0; i < ITEM_SLOTS; i++) {
        for (j = i + 1; j < ITEM_SLOTS; j++) {
            if ((list[i] & ITEM_ID) ==
                (list[j] & ITEM_ID)) {
                count = (list[i] >> 9) + (list[j] >> 9);
                if (count > ITEM_MAX) {
                    count = ITEM_MAX;
                }
                list[i] = (count << 9) +
                                     (list[i] & ITEM_ID);
                list[j] = 0;
            }
        }
    }

    for (i = 0; i < ITEM_SLOTS; i++) {
        if ((list[i] >> 9) == 0 ||
            (list[i] & ITEM_ID) == 0) {
            list[i] = 0;
        }
    }

    for (i = 0; i < ITEM_SLOTS; i++) {
        if (list[i] == 0) {
            for (j = i + 1; j < ITEM_SLOTS; j++) {
                if (list[j] != 0) {
                    list[i] = list[j];
                    list[j] = 0;
                    break;
                }
            }
        }
    }
}

/* Adds to the staging list, taking the first slot with an empty half if the
   item is not there yet. Counts stop at 99.
   The base is spelled out again inside the loop on purpose - do not tidy it
   away. */
void ItemsAddPending(u_short id, short count)
{
    u_short *list;
    u_short *slot;
    short    i;
    short    n;
    int      packed;

    list = g_items_pending;
    i = ItemsFindPending(id);
    if (i < 0) {
        for (;;) {
            short j;

            i++;
            list = g_items_pending;
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
}

/* Subtracts n. Nothing checks that the entry exists first. */
void ItemsRemovePending(u_short id, short n)
{
    u_short *list;
    u_short *p;
    short    count;

    list = g_items_pending;
    p = &list[ItemsFindPending(id)];
    count = (*p >> 9) - n;
    *p = (id & ITEM_ID) + count * 0x200;
}

/* Returns the index of the first entry naming `id`, or -1 if the item is not
   held. Only the id half is compared, so a count of zero still matches. */
short ItemsFind(u_short id)
{
    u_short *list;
    short    i;

    list = g_items;
    for (i = 0; i < ITEM_SLOTS; i++) {
        if ((list[i] & ITEM_ID) == id) {
            return i;
        }
    }
    return -1;
}

/* The same search against the staging list, so a pickup can be added to an
   entry that is already waiting to be merged. */
short ItemsFindPending(u_short id)
{
    u_short *list;
    short    i;

    list = g_items_pending;
    for (i = 0; i < ITEM_SLOTS; i++) {
        if ((list[i] & ITEM_ID) == id) {
            return i;
        }
    }
    return -1;
}

/* Merges the staging list into g_items: same id if it is already there, first
   slot with an empty half otherwise. Both searches are written out rather than
   calling ItemsFind, which is how the original has it.

   This does not come out of the C yet, so the overlays take it from asm; it is
   kept here for the progress build only. */
#ifdef NON_MATCHING
void ItemsCommitPending(void)
{
    u_short *src;
    u_short *dst;
    short    i;
    short    j;

    i = 0;
    do {
        src = &g_items_pending[i];
        if (*src != 0) {
            j = 0;
            do {
                if ((g_items[j] & ITEM_ID) == (*src & ITEM_ID)) {
                    goto found;
                }
                j = j + 1;
            } while (j < ITEM_SLOTS);
            j = -1;
        found:
            if (j < 0) {
                do {
                    j = j + 1;
                    dst = &g_items[j];
                    if ((*dst & ITEM_ID) == 0) {
                        break;
                    }
                } while ((*dst >> 9) != 0);
                *dst = *src;
            } else {
                g_items[j] = *src;
            }
        }
        i = i + 1;
    } while (i < ITEM_SLOTS);
}
#endif
