/* Persona 1 (JP) - searching the Personas on offer.  BTLP only.
 *   0x80069780 BtlOfferFind        0x800697BC BtlOfferFree
 *   0x800743B0 BtlOfferMarkStrong  0x80074438 BtlOfferMarkWeak
 *
 * Three slots, and two ways of looking through them: by the Persona an offer
 * would hand over, and for one that is not spoken for yet.
 *
 * The search for a free slot has no way out if all three are taken - it spins -
 * so whoever calls it has already made sure one is free.
 */
#include <types.h>
#include <persona/btlp/offer.h>

/* The mask at +0 is a summary of the four gauges at two levels, and these two
   keep it up to date - one level each, four bits each. Each returns how many
   gauges it found at its level across all three offers.

   Both arms use a compound assignment on the record through a pointer local,
   which is what merges the two stores into the one the original has. */
int BtlOfferMarkStrong(void)
{
    BtlOffer *o;
    short    *m;
    int       i;
    int       n;
    int       slot;

    n = 0;
    o = g_btl_offer;
    slot = 0;
    do {
        if (o->used != 0) {
            m = o->mood;
            i = 0;
            do {
                if (*m >= BTL_MOOD_STRONG) {
                    n++;
                    o->kinds |= 1 << i;
                } else {
                    o->kinds &= ~(1 << i);
                }
                i++;
                m++;
            } while (i < BTL_MOODS);
        }
        slot++;
        o++;
    } while (slot < BTL_OFFERS);
    return n;
}

int BtlOfferMarkWeak(void)
{
    BtlOffer *o;
    short    *m;
    int       which;
    int       i;
    int       n;
    int       slot;

    n = 0;
    o = g_btl_offer;
    slot = 0;
    do {
        if (o->used != 0) {
            m = o->mood;
            i = 0;
            which = BTL_MOODS;
            do {
                if (*m >= BTL_MOOD_WEAK) {
                    n++;
                    o->kinds |= 1 << which;
                } else {
                    o->kinds &= ~(1 << which);
                }
                m++;
                i++;
                which++;
            } while (i < BTL_MOODS);
        }
        slot++;
        o++;
    } while (slot < BTL_OFFERS);
    return n;
}

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
