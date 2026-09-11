/* Persona 1 (JP) - the turn order a scripted fight is given.  BTLP only.
 *   0x80098C44 BtlResetTurnOrder
 *
 * Every fighter's action is put back to "spent" and the round's order is read
 * out of a script rather than rolled for: one 14-byte row per encounter, each
 * entry a slot to act, ended by 0xFF. The answer is how many turns the row
 * held.
 *
 * An entry with its top bit set names a character by key rather than by slot,
 * so a scripted line always reaches the same person however the party is
 * arranged; the search is over the five party records, and an entry naming
 * somebody who is not there leaves its place in the order unwritten and still
 * counts. The rows for encounter 17 and up sit past the early ones, and the
 * base below is where the bias the compiler folded lands - encounter 17 is
 * its first row, so the index is not adjusted.
 */
#include <decomp/types.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/round.h>

/* The rows for encounter 17 and up are past the early ones, and the base
   below is where the bias the compiler folded lands - encounter 17 is its
   first row, so the index is not adjusted and the name is the run the
   address happens to fall in. */
extern u_char g_btl_turn_script[];
extern u_char g_btl_affinity_scale[];

#define g_btl_turn_script_late (g_btl_affinity_scale + 0x9E)

#define BTL_TURN_SCRIPT_ROW  14
#define BTL_TURN_SCRIPT_LATE 0x11
#define BTL_TURN_SCRIPT_END  0xFF

/* An entry that names a key rather than a slot, and what is left of it once
   that bit is off. */
#define BTL_TURN_BY_KEY  0x80
#define BTL_TURN_KEY_BITS 0x7F

/* Nobody has an action until the round hands one out. */
#define BTL_ACTION_NONE 0xFF


int BtlResetTurnOrder(void)
{
    u_char *entry;
    u_char *order;
    int     turns;
    int     slot;
    int     key;
    int     none;
    int     i;

    /* Walked as a byte offset rather than as an index: with an index gcc
       keeps the count as well as the offset it derives from it, and the loop
       comes out an instruction longer. The value goes into a local of its own
       so it is set up ahead of the offset, which is the order the image has. */
    none = BTL_ACTION_NONE;
    i = (BTL_ACTORS - 1) * sizeof(BtlActor);
    do {
        ((BtlActor *)((u_char *)g_btl_actors + i))->action = none;
        i -= sizeof(BtlActor);
    } while (i >= 0);

    if (g_btl_encounter < BTL_TURN_SCRIPT_LATE) {
        entry = g_btl_turn_script + g_btl_encounter * BTL_TURN_SCRIPT_ROW;
    } else {
        entry = g_btl_turn_script_late + g_btl_encounter * BTL_TURN_SCRIPT_ROW;
    }

    order = g_btl_turn_order;
    turns = 0;
    while (*entry != BTL_TURN_SCRIPT_END) {
        /* Read once and used twice - the image tests the flag and masks the
           key out of the same register, which a second load would not give. */
        key = *entry;
        if (key & BTL_TURN_BY_KEY) {
            for (slot = 0; slot < BTL_PARTY; slot++) {
                if (g_btl_actors[slot].c.key == (key & BTL_TURN_KEY_BITS)) {
                    *order = slot;
                    break;
                }
            }
        } else {
            *order = key;
        }
        entry++;
        order++;
        turns++;
    }
    return turns;
}
