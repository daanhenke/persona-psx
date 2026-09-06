/* Persona 1 (JP) - re-equipping one group with the best the character has.
 *
 * Compiled into two overlays rather than called across the boundary:
 *   ADV 0x80093B24   S2D 0x8007E908
 *
 * The optimise entry of the equipment screen calls this for one group, or for
 * all seven in turn. What is worn comes off first, so the item already in the
 * slot is back in the bag and can be chosen again; the candidates were listed
 * before that, and the count is kept in a global because CharUnequip runs in
 * between.
 *
 * The same walk as ItemsStrongestUsable, spelled out again rather than called:
 * the entry with the highest ItemDef power wins, and an empty list leaves the
 * group bare.
 */
#include <types.h>
#include <persona/common/char.h>
#include <persona/common/item.h>

/* The staging list, reached by hardcoded address; S2D keeps it 0x20000
   higher. */
#define g_items_pending ((u_short *)(0x800EAE4C + WORK_BIAS))

#define ITEM_ID 0x1FF

extern short g_equip_last;

extern int  ItemsListUsable(short kind, short group);
extern void CharUnequip(u_char slot, u_char group);
extern void CharEquip(u_char slot, u_char group, u_short id);

void CharEquipBest(short slot, short group)
{
    u_short *pend;
    u_short *list;
    u_short  entry;
    u_short  id;
    int      last;
    int      best;
    int      i;

    g_equip_last = ItemsListUsable(g_chars[slot].key - 1, group);
    CharUnequip(slot, group);
    i    = 0;
    best = 0;
    if (g_equip_last >= 0) {
        /* Two readers of the same list: one walks it, one indexes the best so
           far. Folding them into one costs the walk its own base, and the
           count has to be taken between the two. */
        pend = g_items_pending;
        last = g_equip_last;
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
    id = g_items_pending[best] & ITEM_ID;
    if (id != 0) {
        CharEquip(slot, group, id);
    }
}
