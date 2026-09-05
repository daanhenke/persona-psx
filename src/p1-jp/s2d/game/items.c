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
#define ITEM_MAX   99

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

extern short ItemsFindPending(u_short id);

/* Subtracts n from an item's count. Nothing checks the entry exists first. */
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
