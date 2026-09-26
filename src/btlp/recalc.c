/* Persona 1 (JP) - the numbers a character actually fights with.  BTLP only.
 *   0x80085D48 BtlRecalcStats
 *
 * Six values derived from the equipment and the five stats, run once as the
 * battle takes each party member on. Each is a weapon or armour number out of
 * the item table plus a stat, half of a second and a quarter or a fifth of a
 * third - the same shape the field's own CharRecalcStats uses.
 *
 * The gun is the exception: it needs both a gun and ammunition, and without
 * either its two numbers are zero and the character's queued gun command is
 * taken away with them.
 */
#include <decomp/types.h>
#include <persona/common/char.h>
#include <persona/common/item.h>
#include <persona/btlp/actor.h>

/* Equipment slots, in the order Char keeps them. */
#define EQUIP_WEAPON 0
#define EQUIP_GUN    1
#define EQUIP_AMMO   2
#define EQUIP_ARMOUR 3

/* The queued command that needs a gun. */
#define BTL_CMD_SHOOT 3

/* The equipment term is outside the grouped stats, as in the wolf case of
   BtlDeriveBattleStats. The armour totals use a fresh local table base so gcc
   reloads its address after the gun branches. */
void BtlRecalcStats(BtlActor *a)
{
    const ItemDef *weapon;
    const ItemDef *gun;
    const ItemDef *ammo;
    int            armour_atk;
    int            armour_hit;
    const ItemDef *defs;

    weapon = &g_item_defs[a->c.equip[EQUIP_WEAPON]];
    a->c.melee_atk = weapon->power
                     + ((u_char)(a->c.level / 5)
                        + (a->c.stat[STAT_STRENGTH] + a->c.stat[STAT_DEXTERITY] / 2));
    a->c.melee_hit = weapon->rate
                     + ((a->c.stat[STAT_DEXTERITY] + a->c.stat[STAT_AGILITY] / 2)
                        + a->c.stat[STAT_LUCK] / 4);

    if (a->c.equip[EQUIP_GUN] != 0 && a->c.equip[EQUIP_AMMO] != 0) {
        gun  = &g_item_defs[a->c.equip[EQUIP_GUN]];
        ammo = &g_item_defs[a->c.equip[EQUIP_AMMO]];
        a->c.gun_atk = (a->c.stat[STAT_DEXTERITY] / 2
                        + (gun->power + ammo->power))
                       + a->c.stat[STAT_AGILITY] / 4;
        a->c.gun_hit = gun->rate
                       + ((a->c.stat[STAT_DEXTERITY] + a->c.stat[STAT_AGILITY] / 2)
                          + a->c.stat[STAT_LUCK] / 4);
    } else {
        a->c.gun_atk = 0;
        a->c.gun_hit = 0;
        if ((a->c.unk5D & 0xF) == BTL_CMD_SHOOT) {
            a->c.unk5D &= 0xF0;
        }
        if (a->tactic == 1) {
            a->tactic = 0;
        }
    }

    /* These loop scopes emit no branches. Their reference weights keep the
       luck contribution in a3 rather than the second armour index's t0. */
    do
    {
        u_short agility_half;
        int     armour_offset;
        u_char  luck;

        do
        {
            luck = a->c.stat[STAT_LUCK] / 4;
        } while (0);
        /* The half-word local puts the defence's agility load ahead of the
           dexterity and luck loads without sharing the evade's narrow read. */
        agility_half = a->c.stat[STAT_AGILITY] / 2;
        /* Form this byte offset before the table base: the shift then stays
           ahead of the level division's multiply in the second scheduler. */
        armour_offset = a->c.equip[EQUIP_ARMOUR] * sizeof(ItemDef);
        defs = g_item_defs;
        armour_atk = ((const ItemDef *)((const u_char *)defs + armour_offset))->power
                     + defs[a->c.equip[EQUIP_ARMOUR + 1]].power
                     + defs[a->c.equip[EQUIP_ARMOUR + 2]].power
                     + defs[a->c.equip[EQUIP_ARMOUR + 3]].power;
        armour_hit = defs[a->c.equip[EQUIP_ARMOUR]].rate
                     + defs[a->c.equip[EQUIP_ARMOUR + 1]].rate
                     + defs[a->c.equip[EQUIP_ARMOUR + 2]].rate
                     + defs[a->c.equip[EQUIP_ARMOUR + 3]].rate;
        a->c.evade = (a->c.stat[STAT_AGILITY] + a->c.stat[STAT_DEXTERITY] / 2)
                     + luck + armour_hit;
        a->c.defence = (u_char)(a->c.level / 5)
                       + (a->c.stat[STAT_VITALITY] + agility_half)
                       + armour_atk;
    } while (0);
}
