/* Persona 1 (JP) - the better of what a character wears and the best they could.
 *
 * Compiled into two overlays rather than called across the boundary:
 *   ADV 0x80093D5C   DNG 0x8008E630
 *
 * Right behind ItemsStrongestUsable (equipbest.c): the strongest item the
 * character may equip in a group, against what they have on in that slot,
 * by `power`. Defined old-style, so both numbers are narrowed here.
 */
#include <decomp/types.h>
#include <persona/common/char.h>
#include <persona/common/item.h>

extern int     ItemsStrongestUsable(short slot, short group);

u_short EquipPickStronger(member, group)
    short member;
    short group;
{
    Char   *chars = g_chars;
    int     best;
    int     cur;

    best = (u_short)ItemsStrongestUsable(member, group);
    switch (group) {
    case 0:
        cur = chars[member].equip[0];
        break;
    case 1:
        cur = chars[member].equip[1];
        break;
    case 2:
        cur = chars[member].equip[2];
        break;
    case 3:
        cur = chars[member].equip[3];
        break;
    case 4:
        cur = chars[member].equip[4];
        break;
    case 5:
        cur = chars[member].equip[5];
        break;
    case 6:
        cur = chars[member].equip[6];
        break;
    }
    if (g_item_defs[best].power > g_item_defs[cur].power) {
        return best;
    }
    return cur;
}
