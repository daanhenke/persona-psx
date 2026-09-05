/* Persona 1 (JP) - "does this apply to anybody in the party?".
 *
 * Compiled into three overlays rather than called across the boundary:
 *                     DNG         ADV         S2D
 *   PartyAnyStatus    0x8008F29C  0x800949C8  0x8007F7AC
 *   PartyAnyBelowMax  0x8008F35C  0x80094A88  0x8007F86C
 *
 * The item menu greys out a recovery item nobody would benefit from, which is
 * what these two answer.
 */
#include <types.h>

/* Highest occupied party slot, cached from PartyLastSlot() when the overlay
   starts up. Each overlay keeps its own copy in its own work area. */
extern u_char g_party_last;

extern u_char CharHasStatus(u_char slot, u_char status);
extern u_char CharBelowMax(u_char slot, u_char kind);

/* Both loops run to completion rather than stopping at the first hit - the
   count is re-read every iteration because the callee might have changed it. */
u_char PartyAnyStatus(u_char status)
{
    int    slot;
    u_char any;

    any = 0;
    for (slot = 0; slot <= g_party_last; slot++) {
        if (CharHasStatus(slot, status)) {
            any = 1;
        }
    }
    return any;
}

u_char PartyAnyBelowMax(u_char kind)
{
    int    slot;
    u_char any;

    any = 0;
    for (slot = 0; slot <= g_party_last; slot++) {
        if (CharBelowMax(slot, kind)) {
            any = 1;
        }
    }
    return any;
}
