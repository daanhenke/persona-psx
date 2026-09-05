/* Persona 1 (JP) - the item lists.
 *
 * Compiled into three overlays rather than called across the boundary:
 *              DNG         ADV         S2D
 *   Find       0x80092918  0x8008E848  0x80082E2C
 *   FindPend   0x80092978  0x8008E8A8  0x80082E8C
 *   Compact    0x80092B30  0x8008EA90  0x80083044
 *   AddPend    0x800927C8  0x8008E6F8  0x80082CDC
 *
 * An entry packs both halves into one u16: the low 9 bits are the item id and
 * the top 7 the count, so a slot counts as in use only when neither half is
 * zero. Both lists are 0x17F entries.
 *
 * g_items is the persistent list - it sits in the save-game work area beside
 * the event flags and the play-time counter. g_items_pending is a staging list
 * in the overlay work area; the routine at 0x8008E908 walks it and merges each
 * entry into g_items, matching by id or taking the first free slot.
 */
#include <types.h>

/* Both lists are reached by hardcoded address rather than through a linker
   symbol. */
#define g_items         ((u_short *)0x801F267C)
#define g_items_pending ((u_short *)0x800EAE4C)

#define ITEM_COUNT 0x17F
#define ITEM_ID    0x1FF

/* Returns the index of the first entry naming `id`, or -1 if the item is not
   held. Only the id half is compared, so a count of zero still matches. */
short ItemsFind(u_short id)
{
    u_short *list;
    short    i;

    list = g_items;
    for (i = 0; i < ITEM_COUNT; i++) {
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
    for (i = 0; i < ITEM_COUNT; i++) {
        if ((list[i] & ITEM_ID) == id) {
            return i;
        }
    }
    return -1;
}

/* Zeroes any half-filled slot in the persistent list - an entry with a count
   but no id, or an id but no count - so the whole word reads as free. */
void ItemsCompact(void)
{
    u_short *p;
    int      i;

    for (i = 0, p = g_items; i < ITEM_COUNT; i++) {
        if ((*p >> 9) == 0 || (*p & ITEM_ID) == 0) {
            *p = 0;
        }
        p++;
    }
}

extern short ItemsFindPending(u_short id);

#define ITEM_MAX 99

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

/* Merges the staging list into g_items: same id if it is already there, first
   slot with an empty half otherwise. Both searches are written out rather than
   calling ItemsFind, which is how the original has it. */
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
            } while (j < ITEM_COUNT);
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
    } while (i < ITEM_COUNT);
}

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

    for (i = 0; i < ITEM_COUNT; i++) {
        for (j = i + 1; j < ITEM_COUNT; j++) {
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

    for (i = 0; i < ITEM_COUNT; i++) {
        if ((list[i] >> 9) == 0 ||
            (list[i] & ITEM_ID) == 0) {
            list[i] = 0;
        }
    }

    for (i = 0; i < ITEM_COUNT; i++) {
        if (list[i] == 0) {
            for (j = i + 1; j < ITEM_COUNT; j++) {
                if (list[j] != 0) {
                    list[i] = list[j];
                    list[j] = 0;
                    break;
                }
            }
        }
    }
}

/* Empties the staging list. The item menus refill it with the subset of
   g_items they are about to page through, so it doubles as the working list
   for a menu as well as the pending-merge buffer. */
void ItemsClearPending(void)
{
    u_short *p;
    int      i;

    i = ITEM_COUNT - 1;
    p = &g_items_pending[ITEM_COUNT - 1];
    for (; i >= 0; i--) {
        *p-- = 0;
    }
}
