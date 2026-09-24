/* Persona 1 (JP) - a fighter picked at random from a side, leaving one out.
 * BTLP only.
 *   0x800962D8 BtlPickOtherMember  0x800963F4 BtlPickOtherEnemy
 *
 * The second pair of random pickers, which charm turns a fighter round with:
 * each walks its own side, writes down every slot that could be hit other than
 * the one it is handed - the charmed fighter's own - and answers one of them
 * off a single call to rand(). Unlike BtlPickRandomMember and
 * BtlPickRandomEnemy these do not ask whether a slot was made pickable, keep a
 * scratch list per side, and answer -1 rather than a slot when there is nobody
 * to pick.
 *
 * Both step over anyone down or out of the fight, and anyone lifted off the
 * floor or held by a puppet string. The pick is tested against the slot left
 * out once more on the way back, though the walk has already made sure it
 * cannot be.
 */
#include <decomp/types.h>
#include <rand.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/round.h>

/* What the two walks write down. */
extern u_char g_btl_other_members[BTL_PARTY];
extern u_char g_btl_other_enemies[BTL_ENEMIES];

/* The tests read like the macros they were: the ailment is taken into a
   `signed char` inside the test it is first compared in, and lifted and
   puppet are two tests of their own rather than one range. gcc folds the two
   into the range the image has, and the byte-wide local is what costs the
   eight bytes of frame nothing reads and puts the copy in the first branch's
   delay slot. The pick at the end reads its table entry twice, once to test
   and once to answer, which is what keeps the index in the walk's counter. */
int BtlPickOtherMember(int slot)
{
    signed char status;
    int         n;
    int         i;

    i = 0;
    n = 0;
    do {
        if (g_btl_actors[i].c.key != 0
            && (status = g_btl_actors[i].c.status) != BTL_STATUS_DOWN
            && (g_btl_actors[i].flags & BTL_ACTOR_OUT) == 0
            && status != BTL_STATUS_LIFTED && status != BTL_STATUS_NOINPUT
            && i != slot) {
            g_btl_other_members[n] = i;
            n++;
        }
        i++;
    } while (i < BTL_PARTY);

    if (n == 0) {
        return -1;
    }
    i = rand() % n;
    if (g_btl_other_members[i] == slot) {
        return -1;
    }
    return g_btl_other_members[i];
}

int BtlPickOtherEnemy(int slot)
{
    signed char status;
    int n;
    int i;

    i = BTL_PARTY;
    n = 0;
    do {
        if (g_btl_actors[i].c.key != 0
            && (status = g_btl_actors[i].c.status) != BTL_STATUS_DOWN
            && (g_btl_actors[i].flags & BTL_ACTOR_OUT) == 0
            && status != BTL_STATUS_LIFTED && status != BTL_STATUS_NOINPUT
            && i != slot) {
            g_btl_other_enemies[n] = i;
            n++;
        }
        i++;
    } while (i < BTL_ACTORS);

    if (n == 0) {
        return -1;
    }
    i = rand() % n;
    if (g_btl_other_enemies[i] == slot) {
        return -1;
    }
    return g_btl_other_enemies[i];
}
