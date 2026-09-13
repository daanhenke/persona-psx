/* Persona 1 (JP) - the two ailments that turn a turn into a swing.  BTLP only.
 *   0x80095A94 BtlAilmentTurnCounter  0x80095B7C BtlAilmentTurnBarsak
 *
 * Entries 20 and 21 of g_btl_ailment_turn, and the only two that hand the
 * fighter a target rather than taking its turn away.
 *
 * The counter answers one particular blow: the slot that dealt it is already
 * on the record, so all this does is check the weapon can still reach that
 * slot and that the slot is pickable, put the fighter's own order and target
 * mask somewhere the turn can be put back from, and aim at whoever it is
 * answering. Anything in the way puts the counter away and cancels the turn.
 *
 * Barsak has nobody in mind. It marks out everything the weapon reaches, walks
 * the enemies and switches off every one the weapon would do nothing to -
 * either because the element is turned aside or because the fighter is behind
 * a ward - and then swings at the slowest of whatever is left. With nothing
 * left it answers 4 rather than nought, which is the one place in the table
 * that is not a cancel.
 */
#include <decomp/types.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/damage.h>
#include <persona/btlp/round.h>
#include <persona/btlp/status.h>
#include <persona/common/item.h>

/* The mask is kept as a whole word here and read back as a half in
   memberact.c, so each unit declares it for itself - the image has a word
   store on this side and a halfword load on that one. */
extern u_long g_btl_counter_targets;

/* The wards that make a fighter not worth swinging at, alongside the affinity
   turning the element aside. */
#define AIL_BARSAK_WARDED 0x1900

void BtlAilmentTurnCounter(BtlActor *a, u_char *act)
{
    if (a->counter == 0) {
        return;
    }
    if (BtlMarkMoveArea(a, g_item_defs[a->c.equip[0]].area,
                        g_item_defs[a->c.equip[0]].swing) >= 0
        && g_btl_actors[a->counter_slot].pickable != 0) {
        g_btl_counter_order = a->order;
        g_btl_counter_targets = a->targets;
        a->order = a->counter_slot;
        a->targets = 1 << a->counter_slot;
        *act = AIL_ACT_AIMED;
        return;
    }
    a->counter = 0;
    *act = 0;
}

void BtlAilmentTurnBarsak(BtlActor *a, u_char *act)
{
    /* One scratch serves both: the affinity's out-parameter first, and then
       the slot the swing settles on. */
    int damage;
    int i;

    if (BtlMarkMoveArea(a, g_item_defs[a->c.equip[0]].area,
                        g_item_defs[a->c.equip[0]].swing) >= 0) {
        i = 0;
        do {
            damage = 0;
            if (g_btl_combatants[i].pickable != 0) {
                if (BtlApplyAffinity(&damage, g_item_defs[a->c.equip[0]].element,
                                     g_btl_combatants[i].c.unk5C) >= 0
                    && (g_btl_combatants[i].flags & AIL_BARSAK_WARDED) == 0) {
                    i++;
                    continue;
                }
                g_btl_combatants[i].pickable = 0;
            }
            i++;
        } while (i < BTL_ENEMIES);
        /* The slot goes into the same scratch the affinity used, straight
           off the call: through a local of its own the store misses the
           branch's delay slot and gcc pads it. */
        damage = BtlSlowestOrder();
        if (damage >= 0) {
            a->order = (u_char)damage + BTL_PARTY;
            a->targets = BtlPickableMask();
            *act = AIL_ACT_AIMED;
            return;
        }
    }
    *act = AIL_ACT_NONE;
}
