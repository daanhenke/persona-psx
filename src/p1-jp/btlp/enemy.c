/* Persona 1 (JP) - is the fight still on?  BTLP only.
 *   0x8006F114 BtlAnyEnemy
 *
 * The enemies occupy the nine actor slots after the party's five, and a record
 * nobody occupies has no Char key. So the question of whether the battle is
 * over is the question of whether any of those nine still carries one.
 */
#include <types.h>
#include <persona/btlp/actor.h>

/* Reached through its own symbol rather than off the party's base, which is
   how the original addresses it. */
extern BtlActor g_btl_enemies[];

int BtlAnyEnemy(void)
{
    BtlActor *a;
    int       i;

    a = g_btl_enemies;
    i = 0;
    do {
        if (a->c.key != 0) {
            return 1;
        }
        i++;
        a++;
    } while (i < BTL_ENEMIES);
    return 0;
}
