/* Persona 1 (JP) - closing the gaps in the party.  ADV only.
 *   0x800ADC30 PartyCompact   0x800ADCB8 PartyFindSlot
 *
 * One of the four units the party source is in: PartyLastSlot is in party.c,
 * the block copy in copyshorts.c and the key search in partyfind.c. The slots
 * themselves are in persona/common/status.h.
 */
#include <decomp/types.h>
#include <persona/common/char.h>
#include <persona/common/status.h>

/* Slides the slots down over the empty ones, so the members the menus draw are
   contiguous. PersonaStockCompact is the same routine over the Persona stock. */
void PartyCompact(void)
{
    u_char *party;
    u_char  i;
    u_char  j;

    party = g_party;
    for (i = 0; i < CHAR_COUNT; i++) {
        if (party[i] == PARTY_EMPTY) {
            for (j = i + 1; j < CHAR_COUNT; j++) {
                if (party[j] != PARTY_EMPTY) {
                    party[i] = party[j];
                    party[j] = PARTY_EMPTY;
                    break;
                }
            }
        }
    }
}

/* The slot holding a given character, or 0xFF if they are not in the party. */
u_char PartyFindSlot(u_char chr)
{
    u_char i;

    for (i = 0; i < 5; i++) {
        if (g_party[i] == chr) {
            return i;
        }
    }
    return 0xFF;
}
