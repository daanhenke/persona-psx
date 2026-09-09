/* Persona 1 (JP) - the last occupied party slot.
 *
 * Compiled into three overlays rather than called across the boundary:
 *   DNG @ 0x8008B86C   ADV @ 0x8007D2DC   S2D @ 0x8007BCDC
 *
 * The original source is four units in the image, each standing on its own:
 * this one, the block copy in copyshorts.c, and ADV's two searches in
 * partycompact.c and partyfind.c. The slots themselves are in
 * persona/common/status.h.
 */
#include <decomp/types.h>
#include <persona/common/char.h>
#include <persona/common/status.h>

/* Index of the last occupied slot.
 *
 * The loop has no lower bound - it walks down from 4 and stops at the first
 * slot that is not empty, so an entirely empty party would run off the front
 * of the array. That is the original's behaviour, not a transcription slip;
 * every caller reaches it with at least one member present. */
u_char PartyLastSlot(void)
{
    int slot;
    int i;

    slot = CHAR_COUNT - 1;
    for (;;) {
        i = (u_char)slot;
        slot--;
        if (g_party[i] != PARTY_EMPTY) {
            return i;
        }
    }
}
