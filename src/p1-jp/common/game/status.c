/* Persona 1 (JP) - ailments, stat caps, and whether an item is worth using.
 *
 * Compiled into three overlays rather than called across the boundary:
 *                    DNG         ADV         S2D
 *   ItemUsableOn     0x8008F154  0x80094880  0x8007F664
 *   CharHasStatus    0x8008F320  0x80094A4C  0x8007F830
 * Everything here reaches the save-game work area by its shared address, so
 * one source covers all three.
 */
#include <types.h>
#include <persona/common/char.h>

/* One byte per party slot, indexing the character records. */
#define g_party ((u_char *)0x801F256C)

/* Ailments the two cure items in this table clear. */
#define STATUS_CURE_67 13
#define STATUS_CURE_6A 16

/* Selector for CharBelowMax: 0 is current HP against its maximum. */
#define BELOW_HP 0

extern u_char CharBelowMax(u_char slot, u_char kind);

/* Does the party member in this slot carry this ailment? */
u_char CharHasStatus(u_char slot, u_char status)
{
    return g_chars[g_party[slot]].status == status;
}

/* Whether using this item on this party member would do anything.
 *
 * Item ids 0x5F..0x61 restore HP, so they are useful while HP is below its
 * maximum; 0x67 and 0x6A each cure one ailment and are useful only while the
 * member has it. Everything else is not a recovery item and answers no. */
u_char ItemUsableOn(u_char slot, short item)
{
    u_char ok;

    ok = 0;
    switch (item) {
    case 0x5F:
    case 0x60:
    case 0x61:
        ok = CharBelowMax(slot, BELOW_HP);
        break;
    case 0x67:
        ok = CharHasStatus(slot, STATUS_CURE_67);
        break;
    case 0x6A:
        ok = CharHasStatus(slot, STATUS_CURE_6A);
        break;
    }
    return ok;
}
