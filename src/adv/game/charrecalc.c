/* Persona 1 (JP) - the numbers a character fights with.  ADV only.
 *   0x8008FD48 CharRecalcStats
 *
 * Run whenever the equipment, the stats or the equipped Persona change: the
 * Persona's pair is copied in (1 and 1 without one), then the six battle
 * numbers are worked out from the equipment and the stats - an item's power
 * or rate plus one stat, half of a second and a quarter or a fifth of a
 * third, the shape btlp/recalc.c repeats for the battle. The k numbers
 * need a k and ammunition both.
 */
#include <decomp/types.h>
#include <persona/common/char.h>
#include <persona/common/item.h>
#include <persona/common/persona.h>

#define EQUIP_WEAPON 0
#define EQUIP_GUN    1
#define EQUIP_AMMO   2
#define EQUIP_ARMOUR 3   /* four armour slots from here */

#define ITEM(c, slot) defs[(c)->equip[slot]]

void CharRecalcStats(u_char n)
{
    Char          *c;
    const ItemDef *defs;
    int            k;   /* the entry, then the gun           */
    int            v;   /* the Persona, the ammunition, then
                           each armour total                 */

    c = &g_chars[n];
    defs = g_item_defs;
    k = c->entry;
    if (k != CHAR_NO_ENTRY && !c->blocked &&
        g_personas[v = c->list[k]].key != 0) {
        c->unk3A = g_personas[v].unk10;
        c->unk3C = g_personas[v].unk12;
    } else {
        c->unk3A = 1;
        c->unk3C = 1;
    }
    k = c->equip[EQUIP_GUN];
    v = c->equip[EQUIP_AMMO];
    c->melee_atk = ITEM(c, EQUIP_WEAPON).power + c->stat[STAT_STRENGTH]
                 + c->stat[STAT_DEXTERITY] / 2 + c->level / 5;
    c->melee_hit = ITEM(c, EQUIP_WEAPON).rate + c->stat[STAT_DEXTERITY]
                 + c->stat[STAT_AGILITY] / 2 + c->stat[STAT_LUCK] / 4;
    if (k != 0 && v != 0) {
        c->gun_atk = defs[k].power + defs[v].power
                   + c->stat[STAT_DEXTERITY] / 2 + c->stat[STAT_AGILITY] / 4;
        c->gun_hit = defs[k].rate + c->stat[STAT_DEXTERITY]
                   + c->stat[STAT_AGILITY] / 2 + c->stat[STAT_LUCK] / 4;
    } else {
        c->gun_atk = 0;
        c->gun_hit = 0;
    }
    v = ITEM(c, EQUIP_ARMOUR).power + ITEM(c, EQUIP_ARMOUR + 1).power
        + ITEM(c, EQUIP_ARMOUR + 2).power + ITEM(c, EQUIP_ARMOUR + 3).power;
    c->defence = v + c->stat[STAT_VITALITY] + c->stat[STAT_AGILITY] / 2
               + c->level / 5;
    v = ITEM(c, EQUIP_ARMOUR).rate + ITEM(c, EQUIP_ARMOUR + 1).rate
        + ITEM(c, EQUIP_ARMOUR + 2).rate + ITEM(c, EQUIP_ARMOUR + 3).rate;
    c->evade = v + c->stat[STAT_DEXTERITY] / 2 + c->stat[STAT_LUCK] / 4
             + c->stat[STAT_AGILITY];
}
