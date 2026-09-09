/* Persona 1 (JP) - is the fight still on?  BTLP only.
 *   0x8006F114 BtlAnyEnemy
 *
 * The enemies occupy the nine actor slots after the party's five, and a record
 * nobody occupies has no Char key. So the question of whether the battle is
 * over is the question of whether any of those nine still carries one.
 */
#include <decomp/types.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/battle.h>

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
