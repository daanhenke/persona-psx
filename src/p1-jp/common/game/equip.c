/* Persona 1 (JP) - equipping and unequipping.
 *
 * An equipment slot holds an inventory entry, not a bare item id, so putting
 * something on takes it out of the inventory and taking it off gives it back.
 * Both directions go through the pending list rather than g_items directly.
 *
 *                DNG         ADV         S2D
 *   CharEquip    0x80090F24  0x8008CE4C  0x80081430
 *   CharUnequip  0x80090FC8  0x8008CEF0  0x800814D4
 */
#include <types.h>
#include <persona/common/char.h>

#define ITEM_ID 0x1FF

extern void ItemsAddPending(u_short id, u_short count);
extern void ItemsRemovePending(u_short id, u_short count);

void CharEquip(u_char chr, u_char slot, u_short item)
{
    Char *c;

    c = &g_chars[chr];
    switch (slot) {
    case 0:
        c->equip[0] = item;
        break;
    case 1:
        c->equip[1] = item;
        break;
    case 2:
        c->equip[2] = item;
        break;
    case 3:
        c->equip[3] = item;
        break;
    case 4:
        c->equip[4] = item;
        break;
    case 5:
        c->equip[5] = item;
        break;
    case 6:
        c->equip[6] = item;
        break;
    }
    if (item & ITEM_ID) {
        ItemsRemovePending(item & ITEM_ID, 1);
    }
}

void CharUnequip(u_char chr, u_char slot)
{
    Char    *c;
    u_short  item;

    c = &g_chars[chr];
    switch (slot) {
    case 0:
        item = c->equip[0];
        c->equip[0] = 0;
        break;
    case 1:
        item = c->equip[1];
        c->equip[1] = 0;
        break;
    case 2:
        item = c->equip[2];
        c->equip[2] = 0;
        break;
    case 3:
        item = c->equip[3];
        c->equip[3] = 0;
        break;
    case 4:
        item = c->equip[4];
        c->equip[4] = 0;
        break;
    case 5:
        item = c->equip[5];
        c->equip[5] = 0;
        break;
    case 6:
        item = c->equip[6];
        c->equip[6] = 0;
        break;
    }
    if (item & ITEM_ID) {
        ItemsAddPending(item & ITEM_ID, 1);
    }
}
