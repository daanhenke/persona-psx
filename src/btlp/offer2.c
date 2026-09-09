/* Persona 1 (JP) - searching the Personas on offer.  BTLP only.
 *   0x80069780 BtlOfferFind        0x800697BC BtlOfferFree
 *   0x800743B0 BtlOfferMarkStrong  0x80074438 BtlOfferMarkWeak
 *   0x8006E28C BtlOfferRank
 *
 * Three slots, and two ways of looking through them: by the Persona an offer
 * would hand over, and for one that is not spoken for yet.
 *
 * The search for a free slot has no way out if all three are taken - it spins -
 * so whoever calls it has already made sure one is free.
 */
#include <decomp/types.h>
#include <persona/btlp/offer.h>

/* The mask at +0 is a summary of the four gauges at two levels, and these two
   keep it up to date - one level each, four bits each. Each returns how many
   gauges it found at its level across all three offers.

   Both arms use a compound assignment on the record through a pointer local,
   which is what merges the two stores into the one the original has. */
/* Defined in the unit before this one; the prototype is what
   decides how the arguments are converted. */
extern int BtlOfferFind(u_char persona);
extern int BtlOfferFree(void);

int BtlOfferRank(int slot)
{
    BtlOffer *o;
    u_long    kinds;
    int       i;

    /* The offer is picked out before the two refreshes and its mask read once
       after them, rather than being indexed again inside the loop. */
    o = &g_btl_offer[slot];
    BtlOfferMarkStrong();
    BtlOfferMarkWeak();
    kinds = o->kinds;
    i = 0;
    while (i < 8) {
        if (((1 << i) & kinds) != 0) {
            break;
        }
        i++;
    }
    if (i < 4) {
        return 1;
    }
    return (i < 8) << 1;
}
