/* Persona 1 (JP) - the numbers a character derives from what it carries.
 *
 * Compiled into three overlays rather than called across the boundary:
 *                   DNG         ADV         S2D
 *   CharApplyStats  0x80094094  0x8008FFF4  0x80084590
 *
 * stat_base is what the character has grown; stat is what it fights with.
 * This rebuilds stat from stat_base each time anything changes: the five
 * equipment slots that carry stat bonuses add theirs, the equipped Persona
 * raises any stat that falls below its own, and the result is capped at 99.
 *
 * An item packs its five bonuses into two and a half bytes, one nibble each,
 * which is why the same byte is read twice per slot - once shifted down for
 * one stat and once masked for the next.
 *
 * The stats are visited in the order 0, 2, 1, 3, 4 throughout, which is the
 * order the status screen lists them in.
 */
#include <types.h>
#include <persona/common/char.h>
#include <persona/common/item.h>
#include <persona/common/persona.h>

/* `k` holds the first item's bonus byte and is reused further down for the
   Persona index. Both uses want the same register; splitting them into two
   locals does not compile to the same code. */
void CharApplyStats(u_char slot)
{
    Char    *c;
    Persona *p;
    u_int    k;
    u_char   unused[0xC0];

    c = &g_chars[slot];
    c->stat[0] = c->stat_base[0];
    c->stat[2] = c->stat_base[2];
    c->stat[1] = c->stat_base[1];
    c->stat[3] = c->stat_base[3];
    c->stat[4] = c->stat_base[4];
    c->stat[0] +=
          (g_item_defs[c->equip[0]].bonus01 >> 4)
        + (g_item_defs[c->equip[3]].bonus01 >> 4)
        + (g_item_defs[c->equip[4]].bonus01 >> 4)
        + (g_item_defs[c->equip[5]].bonus01 >> 4)
        + (g_item_defs[c->equip[6]].bonus01 >> 4);
    c->stat[2] +=
          (g_item_defs[c->equip[0]].bonus23 >> 4)
        + (g_item_defs[c->equip[3]].bonus23 >> 4)
        + (g_item_defs[c->equip[4]].bonus23 >> 4)
        + (g_item_defs[c->equip[5]].bonus23 >> 4)
        + (g_item_defs[c->equip[6]].bonus23 >> 4);
    k = g_item_defs[c->equip[0]].bonus01;
    c->stat[1] +=
          (k & 0xF)
        + (g_item_defs[c->equip[3]].bonus01 & 0xF)
        + (g_item_defs[c->equip[4]].bonus01 & 0xF)
        + (g_item_defs[c->equip[5]].bonus01 & 0xF)
        + (g_item_defs[c->equip[6]].bonus01 & 0xF);
    c->stat[3] +=
          (g_item_defs[c->equip[0]].bonus23 & 0xF)
        + (g_item_defs[c->equip[3]].bonus23 & 0xF)
        + (g_item_defs[c->equip[4]].bonus23 & 0xF)
        + (g_item_defs[c->equip[5]].bonus23 & 0xF)
        + (g_item_defs[c->equip[6]].bonus23 & 0xF);
    c->stat[4] +=
          (g_item_defs[c->equip[0]].bonus4 >> 4)
        + (g_item_defs[c->equip[3]].bonus4 >> 4)
        + (g_item_defs[c->equip[4]].bonus4 >> 4)
        + (g_item_defs[c->equip[5]].bonus4 >> 4)
        + (g_item_defs[c->equip[6]].bonus4 >> 4);
    if (c->entry != 0xFF && c->blocked == 0) {
        k = c->list[c->entry];
        p = &g_personas[k];
        if (c->stat[0] < p->stat[0]) {
            c->stat[0] = p->stat[0];
        }
        if (c->stat[2] < p->stat[2]) {
            c->stat[2] = p->stat[2];
        }
        if (c->stat[1] < p->stat[1]) {
            c->stat[1] = p->stat[1];
        }
        if (c->stat[3] < p->stat[3]) {
            c->stat[3] = p->stat[3];
        }
        if (c->stat[4] < p->stat[4]) {
            c->stat[4] = p->stat[4];
        }
    }
    if (c->stat[0] > 99) {
        c->stat[0] = 99;
    }
    if (c->stat[2] > 99) {
        c->stat[2] = 99;
    }
    if (c->stat[1] > 99) {
        c->stat[1] = 99;
    }
    if (c->stat[3] > 99) {
        c->stat[3] = 99;
    }
    if (c->stat[4] > 99) {
        c->stat[4] = 99;
    }
}
