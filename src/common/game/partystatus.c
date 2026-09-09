/* Persona 1 (JP) - "does this apply to anybody in the party?".
 *
 * Compiled into three overlays rather than called across the boundary:
 *                     DNG         ADV         S2D
 *   PartyAnyStatus    0x8008F29C  0x800949C8  0x8007F7AC
 *   CharHasStatus     0x8008F320  0x80094A4C  0x8007F830
 *   PartyAnyBelowMax  0x8008F35C  0x80094A88  0x8007F86C
 *
 * The item menu greys out a recovery item nobody would benefit from, which is
 * what the two loops answer. CharHasStatus sits between them in the image, so
 * the three are one object even though the first and last read as a pair.
 * ItemUsableOn, above these in the same unit, is in status.c - a routine
 * between the two that is not worked out yet splits them.
 */
#include <decomp/types.h>
#include <persona/common/char.h>
#include <persona/common/status.h>

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

/* Does the party member in this slot carry this ailment? */
u_char CharHasStatus(u_char slot, u_char status)
{
    return g_chars[g_party[slot]].status == status;
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
