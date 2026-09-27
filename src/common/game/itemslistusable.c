/* Persona 1 (JP) - the items one member may wear in one equipment slot.
 *
 * Compiled into three overlays rather than called across the boundary:
 *                      DNG         ADV         S2D
 *   ItemsListUsable    0x8008DDEC  0x80093534  0x8007E318
 *
 * A unit of its own in ADV and S2D, between the equipment cursor and the
 * item row; DNG's equipment screen (src/dng/ui/equipscreen.c) includes it in
 * place.
 */
#include <decomp/types.h>
#include <persona/common/item.h>

/* The item ids each equipment slot takes, lowest and highest. */
extern short g_equip_id_range[][2];

extern void ItemsCompact(void);
extern void ItemsClearPending(void);

/* Stages into g_items_pending every entry whose id is in `slot`'s range and
   that the character with key `key` may wear; returns how many. */
int ItemsListUsable(short key, short slot)
{
    u_short *src;
    u_short *dst;
    int      mask;
    int      i;
    int      n;
    int      id;
    int      lo;
    int      hi;
    int      unused[2];

    mask = 1 << key;
    ItemsCompact();
    ItemsClearPending();
    i = 0;
    n = 0;
    src = g_items;
    dst = g_items_pending;
    lo = g_equip_id_range[slot][0];
    hi = g_equip_id_range[slot][1];
    for (; i < ITEM_SLOTS; i++) {
        id = *src & ITEM_ID;
        if (id != 0 && id >= lo && id <= hi && (g_item_defs[id].owners & mask)) {
            *dst++ = *src;
            n++;
        }
        src++;
    }
    return n;
}
