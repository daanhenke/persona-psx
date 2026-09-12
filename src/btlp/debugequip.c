/* Persona 1 (JP) - stepping one equipment slot through the item table.
 * BTLP only.
 *   0x800A0498 BtlDebugStepEquip
 *
 * The debug page's fighter editor calls this three ways for one slot: down a
 * step, up a step, and - with neither - all the way to the top. Each walk
 * looks for the next item the character is allowed to hold that belongs to
 * that slot's group, and writes its id into the slot; the two stepping walks
 * stop at the first one they find, and the third keeps going and so leaves
 * the last.
 *
 * Two tables turn the question into a pair of comparisons. g_btl_char_bit
 * makes the character's key one bit, which the item's own owner mask is tested
 * against, and g_btl_equip_kind says which of the seven groups the slot takes,
 * which the item's kind bits are matched against.
 *
 * The three walks are written out rather than shared - the original has all
 * three - and the slot's group is read again each turn round, which is what
 * keeps the table's address where the image has it.
 */
#include <decomp/types.h>
#include <persona/common/char.h>
#include <persona/common/item.h>

/* One bit per Char key, and the group each of the seven slots takes. */
extern const u_short g_btl_char_bit[];
extern const u_short g_btl_equip_kind[];

/* The equipment range of the item table: the walks never leave it. */
#define EQUIP_FIRST 0xA3
#define EQUIP_LAST  0x17E

/* Which bits of ItemDef.unk06 say what an item is worn as. */
#define EQUIP_KIND 0x38

/* Which way the caller asked for. */
#define EQUIP_DOWN (-1)
#define EQUIP_UP   1

void BtlDebugStepEquip(Char *c, int slot, int dir)
{
    u_short *equip;
    u_short *cell;
    const u_short *kind;
    const u_short *char_bit;
    int      id;

    /* The slot's row is taken first and the slot added to it. Reached as
       c->equip[slot] throughout, the 0x20 binds to the index instead and every
       address in here comes out the other way round. */
    equip = c->equip;
    id = *(u_short *)(slot * sizeof(*equip) + (u_int)equip);

    if (dir == EQUIP_DOWN) {
        id--;
        while (id >= EQUIP_FIRST) {
            if ((g_item_defs[id].owners & g_btl_char_bit[c->key]) != 0
                && (g_item_defs[id].unk06 & EQUIP_KIND) == g_btl_equip_kind[slot]) {
                equip[slot] = id;
                return;
            }
            id--;
        }
    } else if (dir == EQUIP_UP) {
        id++;
        while (id <= EQUIP_LAST) {
            if ((g_item_defs[id].owners & g_btl_char_bit[c->key]) != 0
                && (g_item_defs[id].unk06 & EQUIP_KIND) == g_btl_equip_kind[slot]) {
                equip[slot] = id;
                return;
            }
            id++;
        }
    } else {
        id++;
        if (id <= EQUIP_LAST) {
            char_bit = g_btl_char_bit;
            kind = &g_btl_equip_kind[slot];
            cell = (u_short *)(slot * sizeof(*equip) + (u_int)equip);
            do {
                if ((g_item_defs[id].owners & char_bit[c->key]) != 0
                    && (g_item_defs[id].unk06 & EQUIP_KIND) == *kind) {
                    *cell = id;
                }
                id++;
            } while (id <= EQUIP_LAST);
        }
    }
}
