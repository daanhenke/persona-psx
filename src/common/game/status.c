/* Persona 1 (JP) - whether an item is worth using on a party member.
 *
 * Compiled into three overlays rather than called across the boundary:
 *                    DNG         ADV         S2D
 *   ItemUsableOn     0x8008F154  0x80094880  0x8007F664
 *
 * This and the three routines in partystatus.c are one unit in the image, cut
 * in two by a routine between them that is not worked out yet, so the overlays
 * take that stretch from asm. Everything shared - the party table, the caps
 * selector and every prototype - is in persona/common/status.h.
 */
#include <decomp/types.h>
#include <persona/common/char.h>
#include <persona/common/status.h>

/* Ailments the two cure items in this table clear. */
#define STATUS_CURE_67 STATUS_POISON
#define STATUS_CURE_6A STATUS_SICK

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
