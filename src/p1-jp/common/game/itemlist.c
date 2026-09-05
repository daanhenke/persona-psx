/* Persona 1 (JP) - the filtered item list the item menus page through.
 *
 * The builder walks g_items, keeps the entries whose id falls in the category
 * range and whose usability mask allows the current actor, and copies them here
 * in order. This clears it first, so the entries left over from the previous
 * menu do not show through a shorter list.
 *
 *   DNG 0x80092B0C   ADV 0x8008EA6C   S2D 0x80083020
 */
#include <types.h>

#define g_item_list ((u_short *)0x800EAE4C)
#define ITEM_LIST_COUNT 0x17F

void ItemListClear(void)
{
    u_short *p;
    int      i;

    i = ITEM_LIST_COUNT - 1;
    p = &g_item_list[ITEM_LIST_COUNT - 1];
    for (; i >= 0; i--) {
        *p-- = 0;
    }
}
