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
#include <types.h>
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

void BtlRecalcStats(BtlActor *a)
{
    ItemDef *items;
    ItemDef *weapon;
    ItemDef *gun;
    ItemDef *ammo;
    ItemDef *armour;
    int      guard;

    items = g_item_defs;
    weapon = &items[a->c.equip[EQUIP_WEAPON]];
    a->c.melee_atk = a->c.stat[2] / 2 + a->c.stat[0] + a->c.level / 5
                     + weapon->power;
    a->c.melee_hit = a->c.stat[3] / 2 + a->c.stat[2] + a->c.stat[4] / 4
                     + weapon->rate;

    if (a->c.equip[EQUIP_GUN] != 0 && a->c.equip[EQUIP_AMMO] != 0) {
        gun = &items[a->c.equip[EQUIP_GUN]];
        ammo = &items[a->c.equip[EQUIP_AMMO]];
        a->c.gun_atk = ammo->power + gun->power
                       + a->c.stat[2] / 2 + a->c.stat[3] / 4;
        a->c.gun_hit = a->c.stat[3] / 2 + a->c.stat[2] + a->c.stat[4] / 4
                       + gun->rate;
    } else {
        a->c.gun_atk = 0;
        a->c.gun_hit = 0;
        if ((a->c.unk5D & 0xF) == BTL_CMD_SHOOT) {
            a->c.unk5D &= 0xF0;
        }
        if (a->unkC8 == 1) {
            a->unkC8 = 0;
        }
    }

    /* The guard is worked out before the evasion and written after it, which
       is the order the original has. */
    guard = items[a->c.equip[EQUIP_ARMOUR + 3]].power
            + items[a->c.equip[EQUIP_ARMOUR + 2]].power
            + items[a->c.equip[EQUIP_ARMOUR + 1]].power
            + items[a->c.equip[EQUIP_ARMOUR]].power
            + a->c.stat[3] / 2 + a->c.stat[1] + a->c.level / 5;
    a->c.evade = items[a->c.equip[EQUIP_ARMOUR + 3]].rate
                 + items[a->c.equip[EQUIP_ARMOUR + 2]].rate
                 + items[a->c.equip[EQUIP_ARMOUR + 1]].rate
                 + items[a->c.equip[EQUIP_ARMOUR]].rate
                 + a->c.stat[2] / 2 + a->c.stat[3] + a->c.stat[4] / 4;
    a->c.defence = guard;
}
