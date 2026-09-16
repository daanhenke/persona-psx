/* Persona 1 (JP) - the fight's own copy of a fighter's numbers.  BTLP only.
 *   0x80093BA0 BtlDeriveBattleStats
 *
 * Everything that swings reads BtlActor's own six values and its own five
 * stats rather than the Char they came from, and this is what fills them in:
 * the Persona is applied first, the Char's numbers are copied across field for
 * field, and then whatever ailment the fighter is under bends the copy.
 *
 * Only eight of the twenty-four ailments bend anything, so the switch is a
 * table with holes in it and everything else leaves the plain copy alone.
 * Sleep and stone take the defence down by eighths, blindness takes both hit
 * numbers down by fifths a level at a time, bad luck cuts the luck stat in
 * half - or to one past the first level - and rebuilds the three numbers luck
 * feeds, poison halves both attack numbers, the two spell marks lift a number
 * by a quarter, and the wolf adds a share of every stat by the moon and then
 * rebuilds all six numbers from them.
 *
 * The arithmetic those arms do is in doubles, which is why the soft-float
 * helpers turn up in an overlay that has no other use for them.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <persona/common/char.h>
#include <persona/common/item.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/status.h>

/* The armour slots, which are weighed together. */
#define EQUIP_WEAPON 0
#define EQUIP_GUN    1
#define EQUIP_AMMO   2
#define EQUIP_ARMOUR 3

/* What each level of blindness leaves of a hit number, out of five. */
#define BLIND_OUT_OF 5.0
#define BLIND_LEFT   4

/* What sleep and stone leave of the defence, out of eight. */
#define SLEEP_OUT_OF 8.0
#define SLEEP_LEVEL0 7.0
#define SLEEP_LEVEL1 6.0
#define SLEEP_LEVEL2 4.0
#define STONE_LEFT   7.0

/* What the two spell marks add to the number they lift. */
#define MARK_SHARE 4.0

extern void BtlApplyPersona(BtlActor *a);

/* 99.58%: every instruction is the image's and in its order; what is left is
   which register holds what in the bad-luck and wolf arms, and the pair of
   equipment ids in the prologue - the image puts the weapon's in $t1 and the
   gun's in $t0 and forms the gun's pointer first, where gcc here does the
   weapon's first and gets them the other way round, which carries through to
   the armour totals. Twenty-odd statement and operand orders have been tried;
   each buys two or three rows and none reaches the end. */
#ifdef NON_MATCHING
void BtlDeriveBattleStats(BtlActor *a)
{
    const ItemDef *defs;
    const ItemDef *weapon;
    const ItemDef *gun;
    int            luck;
    int            aim;
    int            armour_atk;
    int            armour_hit;
    double         d;

    BtlApplyPersona(a);
    defs       = g_item_defs;
    weapon     = &defs[a->c.equip[EQUIP_WEAPON]];
    gun        = &defs[a->c.equip[EQUIP_GUN]];
    armour_atk = defs[a->c.equip[EQUIP_ARMOUR]].power
                 + defs[a->c.equip[EQUIP_ARMOUR + 1]].power
                 + defs[a->c.equip[EQUIP_ARMOUR + 2]].power
                 + defs[a->c.equip[EQUIP_ARMOUR + 3]].power;
    armour_hit = defs[a->c.equip[EQUIP_ARMOUR]].rate
                 + defs[a->c.equip[EQUIP_ARMOUR + 1]].rate
                 + defs[a->c.equip[EQUIP_ARMOUR + 2]].rate
                 + defs[a->c.equip[EQUIP_ARMOUR + 3]].rate;
    a->melee_atk = a->c.melee_atk;
    a->melee_hit = a->c.melee_hit;
    a->gun_atk   = a->c.gun_atk;
    a->gun_hit   = a->c.gun_hit;
    a->defence   = a->c.defence;
    a->evade     = a->c.evade;
    a->unk3A     = a->c.unk3A;
    a->unk3C     = a->c.unk3C;
    a->stat[STAT_STRENGTH]  = a->c.stat[STAT_STRENGTH];
    a->stat[STAT_VITALITY]  = a->c.stat[STAT_VITALITY];
    a->stat[STAT_DEXTERITY] = a->c.stat[STAT_DEXTERITY];
    a->stat[STAT_AGILITY]   = a->c.stat[STAT_AGILITY];
    a->stat[STAT_LUCK]      = a->c.stat[STAT_LUCK];

    switch ((signed char)a->c.status) {
    case BTL_STATUS_SLEEP:
        d = (double)a->c.defence;
        switch ((signed char)a->c.ail_level) {
        case 0:
            a->defence = d / SLEEP_OUT_OF * SLEEP_LEVEL0;
            break;
        case 1:
            a->defence = d / SLEEP_OUT_OF * SLEEP_LEVEL1;
            break;
        case 2:
            a->defence = d / SLEEP_OUT_OF * SLEEP_LEVEL2;
            break;
        }
        break;

    case BTL_STATUS_BLIND:
        d = (double)a->c.melee_hit;
        a->melee_hit = d / BLIND_OUT_OF
                       * (double)(BLIND_LEFT - (signed char)a->c.ail_level);
        d = (double)a->c.gun_hit;
        a->gun_hit = d / BLIND_OUT_OF
                     * (double)(BLIND_LEFT - (signed char)a->c.ail_level);
        break;

    case BTL_STATUS_UNLUCK:
        if ((signed char)a->c.ail_level == 0) {
            a->stat[STAT_LUCK] = a->c.stat[STAT_LUCK] / 2;
        } else {
            a->stat[STAT_LUCK] = 1;
        }
        /* The part the two hit numbers share through a local of its own -
           spelled inline in both the loads in front of them come out in the
           other order. */
        aim          = a->stat[STAT_AGILITY] / 2 + a->stat[STAT_DEXTERITY];
        a->melee_hit = weapon->rate + (a->stat[STAT_LUCK] / 4 + aim);
        /* The gun before the evasion, though the image stores them the other
           way round: written in store order the scheduler keeps the pair of
           stores together and the loads in front of them come out reversed. */
        a->gun_hit = (a->stat[STAT_LUCK] / 4
                      + (a->stat[STAT_DEXTERITY] + a->stat[STAT_AGILITY] / 2))
                     + gun->rate;
        a->evade = a->stat[STAT_LUCK] / 4
                   + (a->stat[STAT_AGILITY] + a->stat[STAT_DEXTERITY] / 2)
                   + armour_hit;
        break;

    case BTL_STATUS_POISON:
        a->melee_atk = a->c.melee_atk / 2;
        a->gun_atk   = a->c.gun_atk / 2;
        break;

    case BTL_STATUS_STONE:
        d          = (double)a->c.defence;
        a->defence = d / SLEEP_OUT_OF * STONE_LEFT;
        d          = (double)a->c.unk3C;
        a->unk3C   = d / SLEEP_OUT_OF * STONE_LEFT;
        break;

    case BTL_STATUS_BARSAK:
        d            = (double)a->c.melee_atk;
        a->melee_atk = d + d / MARK_SHARE;
        d            = (double)a->c.melee_hit;
        a->melee_hit = d + d / MARK_SHARE;
        a->evade     = 0;
        break;

    case BTL_STATUS_MAD:
        d        = (double)a->c.unk3A;
        a->unk3A = d + d / MARK_SHARE;
        break;

    case BTL_STATUS_WOLF:
        if (g_btl_moon_divisor[g_btl_moon] != 0) {
            a->stat[STAT_STRENGTH] +=
                a->stat[STAT_STRENGTH] / g_btl_moon_divisor[g_btl_moon];
            a->stat[STAT_VITALITY] +=
                a->stat[STAT_VITALITY] / g_btl_moon_divisor[g_btl_moon];
            a->stat[STAT_DEXTERITY] +=
                a->stat[STAT_DEXTERITY] / g_btl_moon_divisor[g_btl_moon];
            a->stat[STAT_AGILITY] +=
                a->stat[STAT_AGILITY] / g_btl_moon_divisor[g_btl_moon];
            a->stat[STAT_LUCK] +=
                a->stat[STAT_LUCK] / g_btl_moon_divisor[g_btl_moon];
        }
        a->melee_atk = weapon->power
                       + ((u_char)(a->c.level / 5)
                          + (a->stat[STAT_STRENGTH] + a->stat[STAT_DEXTERITY] / 2));
        a->melee_hit = weapon->rate
                       + ((a->stat[STAT_DEXTERITY] + a->stat[STAT_AGILITY] / 2)
                          + a->stat[STAT_LUCK] / 4);
        if (a->c.equip[EQUIP_GUN] != 0 && a->c.equip[EQUIP_AMMO] != 0) {
            const ItemDef *barrel = &g_item_defs[a->c.equip[EQUIP_GUN]];
            const ItemDef *round  = &g_item_defs[a->c.equip[EQUIP_AMMO]];

            a->gun_atk = a->stat[STAT_AGILITY] / 4
                         + (a->stat[STAT_DEXTERITY] / 2
                            + (barrel->power + round->power));
            a->gun_hit = barrel->rate
                         + ((a->stat[STAT_DEXTERITY]
                             + a->stat[STAT_AGILITY] / 2)
                            + a->stat[STAT_LUCK] / 4);
        }
        /* The quarter of the luck through a local of its own: spelled inline
           it shares the register the melee hit's copy of it is in, and the
           whole tail comes out one register along. */
        luck       = a->stat[STAT_LUCK] / 4;
        /* The defence before the evasion, though the image stores them the
           other way round. */
        a->defence = (u_char)(a->c.level / 5)
                     + (a->stat[STAT_VITALITY] + a->stat[STAT_AGILITY] / 2)
                     + armour_atk;
        a->evade   = luck
                     + (a->stat[STAT_AGILITY] + a->stat[STAT_DEXTERITY] / 2)
                     + armour_hit;
        break;
    }
}
#else
INCLUDE_ASM("btlp/nonmatchings/derivestats", BtlDeriveBattleStats);
#endif
