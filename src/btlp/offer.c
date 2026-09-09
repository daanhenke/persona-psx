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

int BtlOfferFind(u_char persona)
{
    BtlOffer *o;
    int       i;

    o = g_btl_offer;
    i = 0;
    do {
        if (o->persona == persona) {
            return i;
        }
        i++;
        o++;
    } while (i < BTL_OFFERS);
    return BTL_NO_OFFER;
}

int BtlOfferFree(void)
{
    BtlOffer *o;
    int       i;

    o = g_btl_offer;
    i = 0;
    do {
        if (o->used == 0) {
            return i;
        }
        i++;
        o++;
    } while (i < BTL_OFFERS);
    /* Nothing free: the original hangs here rather than reporting it. */
    for (;;) {
    }
}

/* How well an offer is doing. Both halves of the mask are brought up to date
   first, and then the lowest set bit is what answers: bits 0 to 3 mean a gauge
   has passed the higher level, bits 4 to 7 only the lower, and no bit at all
   means neither. */
