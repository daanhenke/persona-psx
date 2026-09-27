/* ADV builds DNG's unit (src/dng/game/itemscommit.c).
 *
 * ADV's commit has the item search written into it where DNG calls
 * ItemsFind: the same loop, expanded inline.
 */
#include <decomp/types.h>
#include <persona/common/item.h>

static inline short ItemsFindInline(u_short id)
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

#define ItemsFind ItemsFindInline
#include "../../dng/game/itemscommit.c"
