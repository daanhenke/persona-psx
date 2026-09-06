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
 */
#include <types.h>
#include <persona/btlp/actor.h>

#define GRID_W     5
#define GRID_CELLS 25
#define CELL_EMPTY 0xFF

/* Encounters that arrange themselves, and where the two tables divide. */
#define ENCOUNTER_KEEP_A 5
#define ENCOUNTER_KEEP_B 8
#define ENCOUNTER_SPLIT  0x11

/* Bytes per character in a table, and per encounter. */
#define PLACE_CHAR      2
#define PLACE_ENCOUNTER 0x14

extern u_char        g_btl_place_party;
extern short         g_btl_encounter;
extern u_char        g_btl_formation[];
/* The grid copied whole: the original moves it as one object, seven words
   at a time, rather than a cell at a time. */
typedef struct {
    u_char cell[GRID_CELLS];
} BtlFormation;

extern BtlFormation  g_btl_formation_saved;
extern const u_char  g_btl_place_lo[];
extern const u_char  g_btl_place_hi[];

void BtlPlaceFormation(void)
{
    const u_char *place;
    u_char *cell;
    char    empty;
    int     slot;
    int     off;
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
        off = 0;
        do {
            key = *((u_char *)&g_btl_actors[0].c.key + off);
            if (key != 0
                && *(signed char *)((char *)&g_btl_actors[0].c.status + off)
                       != BTL_STATUS_DOWN
                && (*(u_long *)((char *)&g_btl_actors[0].flags + off)
                    & BTL_ACTOR_OUT) == 0) {
                if (g_btl_encounter < ENCOUNTER_SPLIT) {
                    place = &g_btl_place_lo[key * PLACE_CHAR
                                            + g_btl_encounter * PLACE_ENCOUNTER];
                } else {
                    place = &g_btl_place_hi[key * PLACE_CHAR
                                            + g_btl_encounter * PLACE_ENCOUNTER];
                }
                g_btl_formation[place[1] * GRID_W + place[0]] = slot;
            }
            slot++;
            off += sizeof(BtlActor);
        } while (slot < BTL_PARTY);
    }
}
