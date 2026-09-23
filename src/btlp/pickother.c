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
#include <decomp/include_asm.h>
#include <rand.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/round.h>

/* What the two walks write down. */
extern u_char g_btl_other_members[BTL_PARTY];
extern u_char g_btl_other_enemies[BTL_ENEMIES];

/* Both 98.66%, one instruction out each, and the same instruction as
   reach.c's BtlPickAiTarget and pickable.c's BtlAnyMemberTargetable: the image
   loads the ailment into a scratch, compares that, and fills the branch's delay
   slot with the move into the register it keeps it in for the range test,
   where gcc here loads straight into that register. Assigning the local after
   the first compare - on a line of its own, in a comma, as a short - comes out
   the same.

   The RTL dumps say what the image did. The comma form already gives the
   right RTL - the scratch compared, then copied into the local after the
   branch - but cse deletes the local unless it outlives the block, and once
   kept (an initialiser before the loop does it) sched1 still hoists the flags
   load over the copy, so the scratch overlaps it and shares its register.
   Declaring the ailment `char` gets the copy, and with it the eight-byte frame
   the `unused` array stands in for (see Matching Nudges, "A char local can
   cost eight frame bytes"), but zero-extends it once more before the range
   test. */
#ifdef NON_MATCHING
int BtlPickOtherMember(int slot)
{
    /* Eight bytes of locals the routine reserves and never writes. */
    long        unused[2];
    signed char status;
    int         n;
    int         i;

    i = 0;
    n = 0;
    do {
        if (g_btl_actors[i].c.key != 0
            && (signed char)g_btl_actors[i].c.status != BTL_STATUS_DOWN
            && (status = (signed char)g_btl_actors[i].c.status,
                (g_btl_actors[i].flags & BTL_ACTOR_OUT) == 0)
            && (u_int)(status - BTL_STATUS_LIFTED) >= 2
            && i != slot) {
            g_btl_other_members[n] = i;
            n++;
        }
        i++;
    } while (i < BTL_PARTY);

    if (n == 0) {
        return -1;
    }
    i = g_btl_other_members[rand() % n];
    if (i == slot) {
        return -1;
    }
    return i;
}
#else
INCLUDE_ASM("btlp/nonmatchings/pickother", BtlPickOtherMember);
#endif

#ifdef NON_MATCHING
int BtlPickOtherEnemy(int slot)
{
    /* Eight bytes of locals the routine reserves and never writes. */
    long unused[2];
    int status;
    int n;
    int i;

    i = BTL_PARTY;
    n = 0;
    do {
        if (g_btl_actors[i].c.key != 0
            && (signed char)g_btl_actors[i].c.status != BTL_STATUS_DOWN
            && (status = (signed char)g_btl_actors[i].c.status,
                (g_btl_actors[i].flags & BTL_ACTOR_OUT) == 0)
            && (u_int)(status - BTL_STATUS_LIFTED) >= 2
            && i != slot) {
            g_btl_other_enemies[n] = i;
            n++;
        }
        i++;
    } while (i < BTL_ACTORS);

    if (n == 0) {
        return -1;
    }
    i = g_btl_other_enemies[rand() % n];
    if (i == slot) {
        return -1;
    }
    return i;
}
#else
INCLUDE_ASM("btlp/nonmatchings/pickother", BtlPickOtherEnemy);
#endif
