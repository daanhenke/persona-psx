/* Persona 1 (JP) - what an attack is worth, and whether it lands.  BTLP only.
 *   0x80092E2C BtlRollHit        0x80092F50 BtlRollCritical
 *   0x80093060 BtlDamageBase     0x800930BC BtlDamageBoosted
 *   0x800931DC BtlApplyAffinity
 *
 * The five together are one attack's arithmetic, in the order it goes through
 * them, and they are the only place the battle rolls dice about a swing.
 *
 * Two of the rolls read a side's averages rather than the two fighters, which
 * is what makes being outnumbered tell: the hit roll spreads whatever headroom
 * is left between an even matchup and a certainty across the swinger's own
 * side, and hands out one share for every fighter it has over the other side.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <rand.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/damage.h>
#include <persona/btlp/sides.h>

/* Whether a swing lands. The two statuses that decide it outright come first:
 * a charmed fighter is swinging at a friend and cannot miss, a panicking one
 * is not aiming at all. Past those, a target that cannot move is hit whatever
 * the numbers say - and the test for that is the whole of a loop the compiler
 * was left to work out is the same answer thirty-two times over. */
#ifdef NON_MATCHING
int BtlRollHit(int ownCount, int accuracy, int selfStatus, int foeCount,
               int evade, int foeStatus)
{
    int chance;
    int spare;
    int raw;
    int difference;
    int helpless;
    int i;

    if (selfStatus == BTL_STATUS_CHARM) {
        return 1;
    }
    if (selfStatus == BTL_STATUS_PANIC) {
        return rand() & 1;
    }

    helpless = 1 << foeStatus;
    for (i = 0; i < 32; i++) {
        if ((helpless & BTL_HELPLESS) != 0) {
            return 1;
        }
    }

    raw = (accuracy - evade) * 2 + BTL_HIT_EVEN;
    if (raw >= 0) {
        chance = raw;
        if (chance > BTL_ROLL_MAX) {
            chance = BTL_ROLL_MAX;
        }
    } else {
        chance = 0;
    }

    raw = chance;
    difference = ownCount - foeCount;
    if (difference >= 0) {
        spare = difference;
        if (spare > BTL_ROLL_MAX) {
            spare = BTL_ROLL_MAX;
        }
    } else {
        spare = 0;
    }

    chance = (BTL_ROLL_MAX - raw) / ownCount * spare;
    chance = (raw + chance) < (rand() & BTL_ROLL_MAX);
    return !chance;
}
#else
INCLUDE_ASM("btlp/nonmatchings/damage", BtlRollHit);
#endif

/* Whether it is a critical. A petrified target is a special case with its own
 * curve - luck alone, taken half again as far, and allowed past a certainty -
 * where an upright one is measured on the average of the three stats that say
 * how well it is paying attention, and can never be worse than one chance in
 * six however far ahead the swinger is. */
int BtlRollCritical(BtlActor *actor, BtlActor *foe)
{
    int raw;
    int chance;

    if ((signed char)foe->c.status == BTL_STATUS_STONE) {
        raw = (int)((actor->stat[STAT_LUCK] - foe->stat[STAT_LUCK] + 0x10)
                    * 1.25);
        if (raw >= 0) {
            chance = raw;
            if (chance > 0x100) {
                chance = 0x100;
            }
        } else {
            chance = 0;
        }
    } else {
        raw = (actor->stat[STAT_AGILITY] + actor->stat[STAT_DEXTERITY]
               + actor->stat[STAT_LUCK]) / 3
              - (foe->stat[STAT_AGILITY] + foe->stat[STAT_DEXTERITY]
                 + foe->stat[STAT_LUCK]) / 3
              + 0x10;
        if (raw >= 0) {
            chance = raw;
            if (chance > 0x29) {
                chance = 0x29;
            }
        } else {
            chance = 0;
        }
    }

    /* Reuse the raw value's local to hold the clamped chance across rand. */
    raw = chance;
    return (rand() & BTL_ROLL_MAX) < raw;
}

/* Raw damage: the swing squared against four times the guard, and never less
   than one however far behind it comes out. */
int BtlDamageBase(int power, int guard)
{
    int damage;

    if (guard == 0) {
        guard = 1;
    }
    damage = power * power / (guard * 4);
    if (damage <= 0) {
        damage = 1;
    }
    return damage;
}

/* The same, with the attack's own percentage on top. The bonus arrives as a
   percentage over and above the whole, so a plain attack passes zero. */
int BtlDamageBoosted(int power, double bonus, int guard)
{
    double damage;

    if (guard == 0) {
        guard = 1;
    }
    damage = (double)(power * power / (guard * 4));
    damage = damage * ((bonus + 100.0) / 100.0);
    return damage <= 0.0 ? 1 : (int)damage;
}

/* What the target's make-up does to the number. The damage is rewritten in
   place and the reaction comes back. */
int BtlApplyAffinity(int *damage, int element, int resist)
{
    double raw;
    u_char bits;
    u_char code;

    /* Taken out of the field before the chart is read: the conversion is a
       call, and the original leaves the whole lookup between it and the
       arithmetic that uses it. */
    raw = *damage;

    bits = g_btl_affinity[resist * BTL_AFFINITY_ROW + element / 2];
    if ((element & 1) != 0) {
        code = bits & 0xF;
    } else {
        code = bits >> 4;
    }

    g_btl_affinity_last = g_btl_affinity_scale[code].scale;
    *damage = raw / BTL_AFFINITY_UNIT * g_btl_affinity_scale[code].scale;

    if (code < BTL_AFFINITY_NULL) {
        return g_btl_affinity_scale[code].react;
    }
    if (code < BTL_AFFINITY_REPEL) {
        return BTL_REACT_NULL;
    }
    return BTL_REACT_REPEL;
}
