/* Persona 1 (JP) - "would casting this do anything right now?".
 *
 * Compiled into three overlays rather than called across the boundary:
 *                DNG         ADV         S2D
 *   SpellUsable  0x8008ED24  0x80094450  0x8007F234
 *
 * The menu greys out a recovery spell nobody in the party would benefit from.
 * Callers pull the id out of a list entry that packs it into the low nine bits
 * with a charge count above it, so the id arrives unpacked.
 *
 * Only the recovery spells answer anything: everything else falls out of the
 * switch with zero. The ones that duplicate a recovery item's effect defer to
 * that item's own test rather than repeating it; the rest ask the party
 * directly. Cases 102 and 104 are always usable.
 */
#include <types.h>

extern u_char ItemUsableAny(short item);
extern u_char PartyAnyBelowMax(u_char kind);
extern u_char PartyAnyStatus(u_char status);
extern u_char CharHasStatus(u_char slot, u_char status);
extern u_char CharBelowMax(u_char slot, u_char kind);

/* The pairs are written as one expression on purpose - splitting them into
   two statements is not the same code. */
u_char SpellUsable(short spell)
{
    u_char r;

    r = 0;
    switch (spell) {
    case 1:
    case 2:
    case 3:
    case 16:
    case 103:
        r = ItemUsableAny(0x62);
        break;
    case 8:
        r = ItemUsableAny(0x67);
        break;
    case 9:
        r = ItemUsableAny(0x6A);
        break;
    case 15:
        r = ItemUsableAny(0x6F);
        break;
    case 18:
        r = ItemUsableAny(0x73);
        break;
    /* The six single-target heals: kinds 4..9 of the same gauge. */
    case 19:
    case 20:
    case 21:
    case 22:
    case 23:
    case 24:
        r = PartyAnyBelowMax(0) + PartyAnyBelowMax(spell - 0xF);
        break;
    case 25:
        r = PartyAnyBelowMax(1) + PartyAnyBelowMax(3);
        break;
    case 26:
        r = PartyAnyBelowMax(9);
        break;
    case 102:
    case 104:
        r = r + 1;
        break;
    case 27:
    case 28:
        r = PartyAnyBelowMax(0) + PartyAnyBelowMax(1);
        break;
    case 29:
        r = PartyAnyBelowMax(0) + PartyAnyBelowMax(1)
            + PartyAnyStatus(0xD) + PartyAnyStatus(0x10);
        break;
    case 34:
        r = PartyAnyBelowMax(1);
        break;
    }
    return r;
}

/* The same question asked of one party member, which is what the target
   cursor greys out. The spell ids agree with SpellUsable's, but only the
   ones that resolve to a party test appear - the item-backed spells are not
   answerable per character, and 102/104 are always allowed. */
u_char CharSpellUsable(u_char slot, short spell)
{
    u_char r;

    r = 0;
    switch (spell) {
    case 1:
    case 2:
    case 3:
    case 16:
        r = CharBelowMax(slot, 0);
        break;
    case 8:
        r = CharHasStatus(slot, 0xD);
        break;
    case 9:
        r = CharHasStatus(slot, 0x10);
        break;
    case 19:
    case 20:
    case 21:
    case 22:
    case 23:
    case 24:
        r = CharBelowMax(slot, 0) + CharBelowMax(slot, spell - 0xF);
        break;
    case 25:
        r = CharBelowMax(slot, 1) + CharBelowMax(slot, 3);
        break;
    case 26:
        r = CharBelowMax(slot, 9);
        break;
    case 27:
    case 28:
        r = CharBelowMax(slot, 0) + CharBelowMax(slot, 1);
        break;
    case 34:
        r = CharBelowMax(slot, 1);
        break;
    }
    return r;
}
