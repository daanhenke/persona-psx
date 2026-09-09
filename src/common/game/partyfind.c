/* Persona 1 (JP) - finding a party member by key.  ADV only.
 *   0x800AF958 PartyFindByKey
 *
 * The last of the four units the party source is in: PartyLastSlot is in
 * party.c, the block copy in copyshorts.c and the compaction in
 * partycompact.c. The slots themselves are in persona/common/status.h.
 */
#include <decomp/types.h>
#include <persona/common/char.h>
#include <persona/common/status.h>

/* The slot holding the character with this key byte, or -1. Event scripts name
   a character by key and need the slot to reach the record. */
short PartyFindByKey(u_char key)
{
    short  i;
    u_char chr;

    for (i = 0; i < CHAR_COUNT; i++) {
        chr = g_party[i];
        if (chr != PARTY_EMPTY && g_chars[chr].key == key) {
            return i;
        }
    }
    return -1;
}
