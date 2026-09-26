/* Persona 1 (JP) - trying a piece of equipment on before wearing it.  DNG only.
 *   0x800946E4 CharPreviewEquip  0x80094DB4 CharPreviewDraw
 *
 * The equipment screen works on a copy of the member: CharPreviewEquip puts
 * the highlighted item into one slot of the copy and works out everything
 * that follows from it - CharApplyStats' stats and CharRecalcStats' battle
 * numbers, written out again here for a Char in hand rather than an index.
 * CharPreviewDraw then prints the copy's numbers beside the member's own,
 * each in the colour and with the arrow of whether it went down or up.
 */
#include <decomp/types.h>
#define TILEMAP_INT_COUNT
#include <persona/common/char.h>
#include <persona/common/item.h>
#include <persona/common/itemname.h>
#include <persona/common/persona.h>
#include <persona/common/tilemap.h>

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

/* ------------------------------------------------------------------------ */

#define AT(row, col) (&g_tilemap1[(row) * MAP_W + (col)])

/* The text atlas has one bank of cells per colour: 0 unchanged, 1 lower,
   2 higher. */
#define BANK_CELLS 0xD7

/* The member's value against the copy's: which colour to draw it in. */
#define COMPARE(was, now)                                                      \
    (col = (was) > (now), (was) < (now) ? col = 2 : 0)

/* The copy's value right-aligned so its last digit lands in `last`, then the
   arrow for `col` in the two cells at `arrow`. */
#define NUMBER(value, width, row, last, arrow)                                 \
    TileMapWriteRowRev(g_hud_digits, AT(row, last),                            \
                       GLYPH_DIGIT0 + col * BANK_CELLS,                        \
                       FormatDecimal(value, g_hud_digits, width));             \
    AT(row, arrow)[0] = arrows[col];                                           \
    AT(row, arrow)[1] = arrows[col] + 1

void CharPreviewDraw(short member, Char *c)
{
    Char   *chars = g_chars;
    u_short arrows[3] = { 0x360, 0x35E, 0x35C };
    Char   *m;
    int     col;

    m = &chars[member];
    TileMapFillRect(AT(0, 5), 0, 8, 1, MAP_W);
    TileMapWriteRow(m->name, AT(0, 5), 0, 8);

    TileMapFillRect(AT(2, 5), 0, 10, CHAR_EQUIP, MAP_W);
    DrawItemName(g_chars[member].equip[0], AT(2, 5), 0, 1);
    DrawItemName(g_chars[member].equip[1], AT(3, 5), 0, 1);
    DrawItemName(g_chars[member].equip[2], AT(4, 5), 0, 1);
    DrawItemName(g_chars[member].equip[3], AT(5, 5), 0, 1);
    DrawItemName(g_chars[member].equip[4], AT(6, 5), 0, 1);
    DrawItemName(g_chars[member].equip[5], AT(7, 5), 0, 1);
    DrawItemName(g_chars[member].equip[6], AT(8, 5), 0, 1);

    TileMapFillRect(AT(3, 24), 0, 2, CHAR_STATS, MAP_W);
    COMPARE(g_chars[member].stat[0], c->stat[0]);
    NUMBER(c->stat[0], 2, 3, 25, 22);
    COMPARE(g_chars[member].stat[1], c->stat[1]);
    NUMBER(c->stat[1], 2, 4, 25, 22);
    COMPARE(g_chars[member].stat[2], c->stat[2]);
    NUMBER(c->stat[2], 2, 5, 25, 22);
    COMPARE(g_chars[member].stat[3], c->stat[3]);
    NUMBER(c->stat[3], 2, 6, 25, 22);
    COMPARE(g_chars[member].stat[4], c->stat[4]);
    NUMBER(c->stat[4], 2, 7, 25, 22);

    TileMapFillRect(AT(2, 34), 0, 3, 6, MAP_W);
    COMPARE(g_chars[member].melee_atk, c->melee_atk);
    NUMBER(c->melee_atk, 3, 2, 36, 32);
    COMPARE(g_chars[member].melee_hit, c->melee_hit);
    NUMBER(c->melee_hit, 3, 3, 36, 32);
    COMPARE(g_chars[member].gun_atk, c->gun_atk);
    NUMBER(c->gun_atk, 3, 4, 36, 32);
    COMPARE(g_chars[member].gun_hit, c->gun_hit);
    NUMBER(c->gun_hit, 3, 5, 36, 32);
    COMPARE(g_chars[member].defence, c->defence);
    NUMBER(c->defence, 3, 6, 36, 32);
    COMPARE(m->evade, c->evade);
    NUMBER(c->evade, 3, 7, 36, 32);
}

/* Two bytes the object carries after the arrow table. Nothing reads them;
   they are here so the rodata after this unit stays in place. */
const u_short D_80064CA2 = 0x4630;
