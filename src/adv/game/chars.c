/* Persona 1 (JP) - putting a character in the party.  ADV only.
 *   0x800ADBC0 PartyAdd
 *
 * One of three units cut out of the same original source: the record searches
 * are in charfind.c and the Persona and list searches in charslots.c. They sit
 * far apart in the overlay, so they were never one object.
 *
 * Five 0x60-byte character records live in the save-game work area, reached
 * through g_party, which holds a record index per party slot.
 */
#include <decomp/types.h>
#include <persona/common/char.h>
#include <persona/common/status.h>

extern u_char PartyFindSlot(u_char chr);

/* Puts a character in the first empty party slot. Nothing checks the party has
   room: a full party gets 0xFF back and writes one past the end. */
void PartyAdd(u_char chr)
{
    g_party[PartyFindSlot(0xFF)] = chr;
}
