/* Persona 1 (JP) - is this actor still in the fight?  BTLP only.
 *   0x80066C28 BtlActorIsDown
 *
 * Two ways to be out, and everything that cares tests both: the ailment code
 * the battle uses for a downed actor, and a flag on the record. The target
 * picker refuses one, the HUD gives it no row, and the cursor steps over it.
 *
 * The battle reads the ailment byte as signed where Char declares it unsigned,
 * so the cast is load-bearing.
 */
#include <types.h>
#include <persona/btlp/actor.h>

int BtlActorIsDown(int slot)
{
    int down;

    if ((signed char)g_btl_actors[slot].c.status == BTL_STATUS_DOWN) {
        return 1;
    }
    /* One local carries the flags and then the answer. Both halves of that are
       load-bearing - split them up and gcc folds the second test and the two
       results into a single set-on-less-than. */
    down = g_btl_actors[slot].flags;
    if ((down & BTL_ACTOR_OUT) != 0) {
        return 1;
    }
    down = 0;
    return down;
}
