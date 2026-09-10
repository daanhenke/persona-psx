/* Persona 1 (JP) - standing the party where the encounter wants.  BTLP only.
 *   0x8008E330 BtlPlaceFormation
 *
 * Most fights use the formation the player arranged, but a scripted one puts
 * the party where it needs them - an ambush from behind, a corridor, a boss
 * that has to be faced from one side. The grid the field handed over is kept
 * so it can be given back afterwards, then rebuilt from the encounter's own
 * table: a column and a row per character, looked up by the Char key rather
 * than by the party slot, so a given character always stands in their place.
 *
 * A member who is absent, down or out of the fight is simply not placed, and
 * their cell stays empty.
 *
 * Two things in here are load-bearing. The record is read by slot rather than
 * walked by a byte offset, which leaves the compiler to make the one offset it
 * needs; and the table entry is reached by stepping to the encounter's block
 * first and then to the character's pair, which is the order the original adds
 * them in.
 */
#include <decomp/types.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/formation.h>

/* Encounters that arrange themselves, and where the two tables divide. */
#define ENCOUNTER_KEEP_A 5
#define ENCOUNTER_KEEP_B 8
#define ENCOUNTER_SPLIT  0x11

void BtlPlaceFormation(void)
{
    const u_char *place;
    u_char *cell;
    char    empty;
    int     slot;
    int     key;

    if (g_btl_place_party != 0 && g_btl_encounter != ENCOUNTER_KEEP_A
        && g_btl_encounter != ENCOUNTER_KEEP_B) {
        g_btl_formation_saved = *(BtlFormation *)g_btl_formation;

        /* The counter the placement loop uses again, which is what keeps both
           loops in the one register. */
        empty = CELL_EMPTY;
        slot = GRID_CELLS - 1;
        cell = &g_btl_formation[GRID_CELLS - 1];
        for (; slot >= 0; slot--) {
            *cell-- = empty;
        }

        slot = 0;
        do {
            key = g_btl_actors[slot].c.key;
            if (key != 0
                && (signed char)g_btl_actors[slot].c.status != BTL_STATUS_DOWN
                && (g_btl_actors[slot].flags & BTL_ACTOR_OUT) == 0) {
                if (g_btl_encounter < ENCOUNTER_SPLIT) {
                    place = g_btl_place_lo
                            + g_btl_encounter * BTL_PLACE_ENCOUNTER
                            + key * BTL_PLACE_CHAR;
                } else {
                    place = g_btl_place_hi
                            + g_btl_encounter * BTL_PLACE_ENCOUNTER
                            + key * BTL_PLACE_CHAR;
                }
                g_btl_formation[place[1] * GRID_W + place[0]] = slot;
            }
            slot++;
        } while (slot < BTL_PARTY);
    }
}

