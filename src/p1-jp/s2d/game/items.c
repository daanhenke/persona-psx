/* Persona 1 (JP) - S2D's copy of ItemsFindPending.  S2D @ 0x80082E8C.
 *
 * Only the staging list moves: g_items sits in the shared save-game work area
 * and is at 0x801F267C in every overlay, so ItemsFind and ItemsCompact are
 * covered by src/p1-jp/common/items.c for all three. g_items_pending is in the
 * overlay work area, which for S2D is 0x20000 higher.
 */
#include <types.h>

#define g_items_pending ((u_short *)0x8010AE4C)

#define ITEM_COUNT 0x17F
#define ITEM_ID    0x1FF

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
