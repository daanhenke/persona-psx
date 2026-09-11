/* Persona 1 (JP) - where the party goes back to.  BTLP only.
 *   0x80099404 BtlSetReturnMap
 *
 * Four bytes per encounter: the map, then the room inside it. The close reads
 * the pair straight into the field's own two globals, so whatever the fight
 * was entered from is what the game returns to.
 */
#include <decomp/types.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/round.h>
#include <persona/main/state.h>

extern u_char g_map_room;


void BtlSetReturnMap(void)
{
    g_map_id   = g_btl_encounter_maps[g_btl_encounter].map;
    g_map_room = g_btl_encounter_maps[g_btl_encounter].room;
}
