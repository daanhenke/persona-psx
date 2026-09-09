/* Persona 1 (JP) - each side's average numbers.  BTLP only.
 *   0x80093308 BtlAverageSides
 *
 * Run once as the battle opens. Both halves are the same shape: sum a level,
 * an agility and a luck over the side's slots, count what was summed, and
 * divide each by the count if anything was counted at all. The agility and
 * luck are the battle's own copies, not the ones on the save record.
 *
 * The two halves do not agree on what counts. A party slot has to be occupied,
 * still alive and not out of the fight; an enemy slot only has to be occupied,
 * so a dead enemy still drags the enemies' averages toward its own numbers.
 *
 * The enemies are reached through the pointer the overlay keeps to them rather
 * than through their own symbol - the same nine records by a different route.
 */
#include <decomp/types.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/sides.h>


void BtlAverageSides(void)
{
    BtlActor *a;
    int       i;

    i = 0;
    g_btl_party_level = 0;
    g_btl_party_agility = 0;
    g_btl_party_luck = 0;
    g_btl_party_counted = 0;
    do {
        if (g_btl_actors[i].c.key != 0
            && (signed char)g_btl_actors[i].c.status != BTL_STATUS_DOWN
            && (g_btl_actors[i].flags & BTL_ACTOR_OUT) == 0) {
            g_btl_party_level += g_btl_actors[i].c.level;
            g_btl_party_agility += g_btl_actors[i].stat[STAT_AGILITY];
            g_btl_party_luck += g_btl_actors[i].stat[STAT_LUCK];
            g_btl_party_counted++;
        }
        i++;
    } while (i < BTL_PARTY);

    i = 0;
    if (g_btl_party_counted != 0) {
        g_btl_party_level = g_btl_party_level / g_btl_party_counted;
        g_btl_party_agility = g_btl_party_agility / g_btl_party_counted;
        g_btl_party_luck = g_btl_party_luck / g_btl_party_counted;
    }

    g_btl_enemy_level = 0;
    g_btl_enemy_agility = 0;
    g_btl_enemy_luck = 0;
    g_btl_enemy_counted = 0;
    /* The level total is cleared twice, and the second one is
       load-bearing: it is what keeps the total in the register the
       loop below reads it from. */
    g_btl_enemy_level = 0;
    a = g_btl_combatants;
    /* Indexed rather than walked: with a++ gcc biases the pointer to the last
       field it reads and every offset comes out negative. */
    do {
        if (a[i].c.key != 0) {
            g_btl_enemy_level += a[i].c.level;
            g_btl_enemy_agility += a[i].stat[STAT_AGILITY];
            g_btl_enemy_luck += a[i].stat[STAT_LUCK];
            g_btl_enemy_counted++;
        }
        i++;
    } while (i < BTL_ENEMIES);

    if (g_btl_enemy_counted != 0) {
        g_btl_enemy_level = g_btl_enemy_level / g_btl_enemy_counted;
        g_btl_enemy_agility = g_btl_enemy_agility / g_btl_enemy_counted;
        g_btl_enemy_luck = g_btl_enemy_luck / g_btl_enemy_counted;
    }
}
