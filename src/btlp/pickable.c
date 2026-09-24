/* Persona 1 (JP) - which fighters a pick may land on.  BTLP only.
 *   0x800C5990 BtlSetPickable  0x800C5A00 BtlSetPartyPickable
 *   0x800C5A90 BtlAnyMemberTargetable
 *
 * A whole side is switched on at once before a pick, and these are the two
 * halves of that: the enemies' and the party's. Both clear the flag on every
 * slot first and then put it back on the ones a cursor may stop at, so a
 * fighter that has fallen since the last pick loses it without anybody having
 * to clear it.
 *
 * The two rule out different things. An enemy is pickable unless its slot is
 * empty or it cannot be given an order; a member also has to be alive and
 * still in the fight. The third routine answers the same question about the
 * party as a whole, and is what the round asks before it bothers to open a
 * pick at all - a member who is only lifted or puppeted does not count, which
 * is the one test the pickable walk does not make.
 */
#include <decomp/types.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/round.h>

void BtlSetPickable(void)
{
    int i;

    i = 0;
    do {
        g_btl_combatants[i].pickable = 0;
        if (g_btl_combatants[i].c.key != 0
            && (signed char)g_btl_combatants[i].c.status
                   != BTL_STATUS_NOINPUT) {
            g_btl_combatants[i].pickable = 1;
        }
        i++;
    } while (i < BTL_ENEMIES);
}

void BtlSetPartyPickable(void)
{
    int status;
    int i;

    i = 0;
    do {
        g_btl_actors[i].pickable = 0;
        if (g_btl_actors[i].c.key != 0) {
            status = (signed char)g_btl_actors[i].c.status;
            if (status != BTL_STATUS_DOWN
                && (g_btl_actors[i].flags & BTL_ACTOR_OUT) == 0
                && status != BTL_STATUS_NOINPUT) {
                g_btl_actors[i].pickable = 1;
            }
        }
        i++;
    } while (i < BTL_PARTY);
}

/* The same tests as pickother.c's pickers, written the same way: the ailment
   taken into a `signed char` as it is first compared, lifted and puppet as two
   tests that gcc folds into one range. The byte-wide local is the eight bytes
   of frame nothing reads, and the copy into it fills the first branch's slot. */
int BtlAnyMemberTargetable(void)
{
    signed char status;
    int  i;

    i = 0;
    do {
        if (g_btl_actors[i].c.key != 0
            && (status = g_btl_actors[i].c.status) != BTL_STATUS_DOWN
            && (g_btl_actors[i].flags & BTL_ACTOR_OUT) == 0
            && status != BTL_STATUS_LIFTED && status != BTL_STATUS_NOINPUT) {
            return 1;
        }
        i++;
    } while (i < BTL_PARTY);
    return 0;
}
