/* Persona 1 (JP) - is the fight still on?  BTLP only.
 *   0x8006F114 BtlAnyEnemy
 *   0x80094CD4 BtlAnyStanding  0x80094D24 BtlBattleOutcome
 *
 * BtlBattleOutcome asks it of the party. Any member present who is not down,
 * not out, not paralysed or stoned and not a puppet still fights, and the
 * answer is 1. Otherwise the answer is 0 and g_btl_party_lost is raised -
 * unless the debug flag says the party cannot be beaten, or a member who is
 * out without 0x80000000 set could still act when brought back, in which case
 * it is lowered again.
 *
 * The enemies occupy the nine actor slots after the party's five, and a record
 * nobody occupies has no Char key. So the question of whether the battle is
 * over is the question of whether any of those nine still carries one.
 */
#include <decomp/types.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/round.h>

/* A member out with this set as well is gone for good. */
#define OUTCOME_GONE 0x80000000

/* Reached through its own symbol rather than off the party's base, which is
   how the original addresses it. */

/* The same nine reached through the pointer the overlay keeps to them. */
/* Defined in the unit before this one; the prototype is what
   decides how the arguments are converted. */
extern int BtlAnyEnemy(void);


/* The stricter question: not just occupied, but still able to fight. */
int BtlAnyStanding(void)
{
    BtlActor *a;
    int       i;

    a = g_btl_combatants;
    i = 0;
    do {
        if (a[i].c.key != 0 &&
            (signed char)a[i].c.status != BTL_STATUS_DOWN) {
            return 1;
        }
        i++;
    } while (i < BTL_ENEMIES);
    return 0;
}

/* The status is a byte local: every int test is made on a widened copy of it,
   and its pseudo is what leaves this leaf an eight-byte frame. */
int BtlBattleOutcome(void)
{
    signed char status;
    int         i;

    for (i = 0; i < BTL_PARTY; i++) {
        if (g_btl_actors[i].c.key != 0) {
            status = g_btl_actors[i].c.status;
            if (status != BTL_STATUS_DOWN
                && (g_btl_actors[i].flags & BTL_ACTOR_OUT) == 0
                && status != BTL_STATUS_PALYZE && status != BTL_STATUS_STONE
                && status != BTL_STATUS_NOINPUT) {
                return 1;
            }
        }
    }
    g_btl_party_lost = g_btl_debug_no_defeat == 0;
    for (i = 0; i < BTL_PARTY; i++) {
        if (g_btl_actors[i].c.key != 0
            && (g_btl_actors[i].flags & BTL_ACTOR_OUT) != 0
            && (g_btl_actors[i].flags & OUTCOME_GONE) == 0
            && g_btl_actors[i].c.status != BTL_STATUS_PALYZE
            && g_btl_actors[i].c.status != BTL_STATUS_STONE
            && (signed char)g_btl_actors[i].c.status != BTL_STATUS_NOINPUT) {
            g_btl_party_lost = 0;
            return 0;
        }
    }
    return 0;
}
