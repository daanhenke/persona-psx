/* Persona 1 (JP) - arming the hit once an effect has played out.  BTLP only.
 *   0x800BFF34 BtlArmHitChain
 *   0x800BFFBC BtlArmHitOne
 *   0x800C003C BtlFinishMoveFx
 *
 * The head of an effect chain calls one of the first two as its script ends:
 * the fight is told which fighter it is resolving, three of its own counters
 * are put on their opening values, and that fighter is taken back out of the
 * acting record's target mask so the same slot is not resolved twice.
 *
 * The two differ in a single value. Which one is called is decided by the mark
 * the record carries - nought for a record standing on its own, 0x10 for the
 * first of a chain - and nothing else about them differs.
 *
 * BtlFinishMoveFx is what every record hands its frame to once the chain is
 * done with.
 */
#include <decomp/types.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/object.h>
#include <persona/btlp/round.h>

/* The fight's own counters, set together as the hit is armed and read by the
   damage step. Only the values written here are known. */
extern short g_btl_hit_mask;
extern short g_btl_hit_walk;
extern short g_btl_hits_left;

/* Which actor the fight is resolving; BtlRollDefeatDrop reads it too. */
extern short g_btl_hit_slot;

extern void func_800C0E54(BtlObj *obj);

void BtlArmHitChain(void)
{
    int slot;

    slot = g_btl_actors[g_btl_actor_turn].order;
    g_btl_hits_left = 9;
    g_btl_hit_walk = -1;
    g_btl_hit_mask = 1;
    g_btl_hit_slot = slot;
    g_btl_actors[g_btl_actor_turn].targets &= ~(1 << slot);
}

void BtlArmHitOne(void)
{
    int slot;

    slot = g_btl_actors[g_btl_actor_turn].order;
    g_btl_hits_left = 1;
    g_btl_hit_walk = -1;
    g_btl_hit_mask = 1;
    g_btl_hit_slot = slot;
    g_btl_actors[g_btl_actor_turn].targets &= ~(1 << slot);
}

void BtlFinishMoveFx(BtlObj *obj)
{
    /* Sixteen bytes of locals the routine never writes - the position every
       other opener in the family declares, left behind here. */
    long pos[3];

    func_800C0E54(obj);
}
