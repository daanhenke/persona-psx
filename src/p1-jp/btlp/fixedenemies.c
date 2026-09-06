/* Persona 1 (JP) - the two encounters whose enemies are written down.  BTLP only.
 *   0x8008E7C4 BtlSpawnFixedEnemies
 *
 * Most fights pick their enemies from an encounter table. Two do not: 0x1D and
 * 0x1E each fill all six slots from a roster held in the overlay, and their
 * only caller runs this for those two ids and no others. Both rosters
 * alternate a pair of species across the six, so the formation reads as two
 * ranks of the same two things.
 *
 * Each slot gets three things: the species goes into the object that is drawn
 * for it, BtlLoadEnemyStats brings in the numbers, and the object is armed
 * with whichever of the model's scripts g_btl_models names.
 */
#include <types.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/object.h>

/* The encounter that gets the first roster; the other one gets the second. */
#define BTL_ENCOUNTER_FIXED_A 0x1D

/* Six species to a roster, in rows of eight. */
#define BTL_FIXED_SLOTS 6

typedef struct {
    /* 0x00 */ u_char script;   /* which of the object's scripts to arm */
    /* 0x01 */ u_char pad01[0x2F];
} BtlModel;                     /* 48 bytes */

/* Two separate objects rather than a table of two rows, which is how the
   original reaches them: both addresses are built in full. */
extern const u_char g_btl_fixed_enemies[];
extern const u_char g_btl_fixed_enemies_alt[];
extern BtlModel     g_btl_models[];
extern short        g_btl_encounter;
extern BtlActor    *g_btl_combatants;

extern void BtlLoadEnemyStats(int slot, int species);
extern void BtlObjSetScript(BtlObj *obj, const u_long *script);

void BtlSpawnFixedEnemies(void)
{
    const u_char *roster;
    const u_char *species;
    int           i;

    roster = g_btl_fixed_enemies;
    if (g_btl_encounter == BTL_ENCOUNTER_FIXED_A) {
        roster = g_btl_fixed_enemies_alt;
    }
    /* The walk starts from a second variable rather than stepping `roster`
       itself. It looks like one assignment too many; it is not. */
    i = 0;
    species = roster;
    do {
        g_btl_combatants[i].obj->kind = *species;
        BtlLoadEnemyStats(i, *species);
        BtlObjSetScript(g_btl_combatants[i].obj,
                        g_btl_combatants[i].obj->scripts
                            [g_btl_models[*species].script]);
        species++;
        i++;
    } while (i < BTL_FIXED_SLOTS);
}
