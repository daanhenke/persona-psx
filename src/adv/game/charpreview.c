/* Persona 1 (JP) - trying a piece of equipment on before wearing it.  ADV.
 *   0x80090644 CharPreviewEquip
 *
 * The equipment screen works on a copy of the member: CharPreviewEquip puts
 * the highlighted item into one slot of the copy and works out everything
 * that follows from it - CharApplyStats' stats and CharRecalcStats' battle
 * numbers, written out again here for a Char in hand rather than an index.
 * DNG's copy (src/dng/game/charpreview.c) also holds the drawing half, which
 * ADV keeps in ui/equipcompare.c.
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

/* A slot past the last leaves the equipment as it is. */
void CharPreviewEquip(Char *c, short slot, u_short item)
{
    const ItemDef *defs;
    Persona       *ps;
    u_int          k;
    int            v;

    defs = g_item_defs;
    ps = g_personas;
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

    c->stat[0] = c->stat_base[0];
    c->stat[2] = c->stat_base[2];
    c->stat[1] = c->stat_base[1];
    c->stat[3] = c->stat_base[3];
    c->stat[4] = c->stat_base[4];
    v = (ITEM(c, 0).bonus01 >> 4) + (ITEM(c, 3).bonus01 >> 4)
      + (ITEM(c, 4).bonus01 >> 4) + (ITEM(c, 5).bonus01 >> 4)
      + (ITEM(c, 6).bonus01 >> 4);
    c->stat[0] += v;
    v = (ITEM(c, 0).bonus23 >> 4) + (ITEM(c, 3).bonus23 >> 4)
      + (ITEM(c, 4).bonus23 >> 4) + (ITEM(c, 5).bonus23 >> 4)
      + (ITEM(c, 6).bonus23 >> 4);
    c->stat[2] += v;
    v = (ITEM(c, 0).bonus01 & 0xF) + (ITEM(c, 3).bonus01 & 0xF)
      + (ITEM(c, 4).bonus01 & 0xF) + (ITEM(c, 5).bonus01 & 0xF)
      + (ITEM(c, 6).bonus01 & 0xF);
    c->stat[1] += v;
    v = (ITEM(c, 0).bonus23 & 0xF) + (ITEM(c, 3).bonus23 & 0xF)
      + (ITEM(c, 4).bonus23 & 0xF) + (ITEM(c, 5).bonus23 & 0xF)
      + (ITEM(c, 6).bonus23 & 0xF);
    c->stat[3] += v;
    v = (ITEM(c, 0).bonus4 >> 4) + (ITEM(c, 3).bonus4 >> 4)
      + (ITEM(c, 4).bonus4 >> 4) + (ITEM(c, 5).bonus4 >> 4)
      + (ITEM(c, 6).bonus4 >> 4);
    c->stat[4] += v;
    k = c->entry;
    if (k != CHAR_NO_ENTRY && c->blocked == 0) {
        v = c->list[k];
        if (c->stat[0] < ps[v].stat[0]) {
            c->stat[0] = ps[v].stat[0];
        }
        if (c->stat[2] < ps[v].stat[2]) {
            c->stat[2] = ps[v].stat[2];
        }
        if (c->stat[1] < ps[v].stat[1]) {
            c->stat[1] = ps[v].stat[1];
        }
        if (c->stat[3] < ps[v].stat[3]) {
            c->stat[3] = ps[v].stat[3];
        }
        if (c->stat[4] < ps[v].stat[4]) {
            c->stat[4] = ps[v].stat[4];
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

    k = c->entry;
    if (k != CHAR_NO_ENTRY && !c->blocked && ps[v = c->list[k]].key != 0) {
        c->mag_atk = ps[v].mag_atk;
        c->mag_def = ps[v].mag_def;
    } else {
        c->mag_atk = 1;
        c->mag_def = 1;
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
