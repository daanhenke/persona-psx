/* Persona 1 (JP) - the item lists.
 *
 * Compiled into three overlays rather than called across the boundary:
 *              DNG         ADV         S2D
 *   Find       0x80092918  0x8008E848  0x80082E2C
 *   FindPend   0x80092978  0x8008E8A8  0x80082E8C
 *   Compact    0x80092B30  0x8008EA90  0x80083044
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
