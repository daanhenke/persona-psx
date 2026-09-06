/* Persona 1 (JP) - the strongest item a character may equip in one slot.
 *
 * Compiled into three overlays rather than called across the boundary:
 *   ADV 0x80093C68   DNG 0x8008E53C   S2D 0x8007EA4C
 *
 * ItemsListUsable stages the candidates for one equipment group in
 * g_items_pending, keyed by the character's kind - their key byte less one -
 * and this keeps the entry whose item has the highest `power`, the byte both
 * the attack and the defence of a group are built from.
 *
 * Entry 0 is the running best from the start, so an empty list gives back
 * whatever happens to be in the first slot rather than nothing.
 */
#include <types.h>
#include <persona/common/char.h>
#include <persona/common/item.h>

/* The staging list, reached by hardcoded address; S2D keeps it 0x20000
   higher. */
#define g_items_pending ((u_short *)(0x800EAE4C + WORK_BIAS))

#define ITEM_ID 0x1FF

extern int   ItemsListUsable(short kind, short group);

u_short ItemsStrongestUsable(short slot, short group)
{
    u_short *pend;
    u_short *list;
    u_short  entry;
    int      last;
    int      best;
    int      i;

    last = ItemsListUsable(g_chars[slot].key - 1, group);
    i    = 0;
    best = 0;
    if (last >= 0) {
        /* Two readers of the same list: one walks it, one indexes the best so
           far. Folding them into one costs the walk its own base. */
        pend = g_items_pending;
        list = g_items_pending;
        do {
            entry = *list++;
            if ((entry & ITEM_ID) != 0 &&
                g_item_defs[pend[best] & ITEM_ID].power <
                    g_item_defs[entry & ITEM_ID].power) {
                best = i;
            }
            i++;
        } while (i <= last);
    }
    return g_items_pending[best] & ITEM_ID;
}
