/* Persona 1 (JP) - the moods a demon starts a negotiation in.  BTLP only.
 *   0x80069508 BtlMoodsFromMoon
 *
 * Three things decide how a demon feels before a word is said.
 *
 * The moon comes first: a new moon lifts one gauge, a full moon lifts three of
 * the four, and the phases either side move one or two of them a little. Then
 * the level gap - a demon far below the party is easier to move one way and
 * harder another, so a fifth of the gap is added to one gauge and taken off
 * another. Last, already holding that Persona lifts two of them.
 *
 * All four are clamped to 0..0x5F afterwards, the range the gauges are read
 * at; BTL_MOOD_STRONG is just inside the top of it. An offer with no enemies,
 * or one not being drawn, is only clamped.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/offer.h>
#include <persona/btlp/talk.h>

/* Bit that marks an offer the battle is showing. */
#define OFFER_SHOWN 0x8000

/* As far as a gauge goes either way. */
#define MOOD_MIN 0
#define MOOD_MAX 0x5F

/* The moon, new through full. */
#define MOON_NEW      0
#define MOON_CRESCENT 2
#define MOON_HALF     6
#define MOON_FULL     8
#define MOON_GIBBOUS  10
#define MOON_WANING   0xE

/* What the level gap and a Persona already held are worth. */
#define MOOD_PER_LEVEL 5
#define MOOD_HELD      3


extern int BtlOfferLevelTest(int test, u_short slot);
extern int BtlStockHolds(const BtlOffer *offer);

/* 97.71%. Four things got it there from 85.11%:
   - the offer is a walking pointer and the counter an int (loop.c makes one
     pointer at the moods for every field);
   - the first gap step is a division by -10, which gcc expands as the negated
     quotient the image has;
   - each clamp has a temporary of its own, so each store can drop into the
     next clamp's delay slot;
   - the crescent jumps into the full moon's store of mood 2 through a short.
   What is left is scheduling. The full moon computes mood 2 before mood 3
   where the image does it after, and the second gap step does mood 0 before
   mood 2. Writing the store as `+= -step`, as `a = a - step`, or through
   `step * -5` does not move either. */
#ifdef NON_MATCHING
void BtlMoodsFromMoon(void)
{
    BtlOffer *offer;
    int slot;
    int step;
    short m;

    offer = g_btl_offer;
    for (slot = 0; slot < BTL_OFFERS; offer++, slot++) {
        if ((offer->flags & OFFER_SHOWN) != 0
            && offer->used != 0) {
            switch (g_btl_moon) {
            case MOON_NEW:
                offer->mood[0] += 10;
                break;
            case MOON_CRESCENT:
            case MOON_WANING:
                m = offer->mood[2] + 5;
                goto set;
            case MOON_HALF:
            case MOON_GIBBOUS:
                offer->mood[1] += 5;
                break;
            case MOON_FULL:
                offer->mood[1] += 10;
                offer->mood[3] += 10;
                m = offer->mood[2] + 10;
            set:
                offer->mood[2] = m;
                break;
            }
            if (BtlOfferLevelTest(3, slot) != 0) {
                step = g_btl_level_gap / -10 * MOOD_PER_LEVEL;
                offer->mood[2] += step;
                offer->mood[1] -= step;
            }
            if (BtlOfferLevelTest(4, slot) != 0) {
                step = g_btl_level_gap / 10 * MOOD_PER_LEVEL;
                offer->mood[1] += step;
                offer->mood[2] -= step;
                offer->mood[0] += step;
            }
            if (BtlStockHolds(offer) != 0) {
                offer->mood[0] += MOOD_HELD;
                offer->mood[3] += MOOD_HELD;
            }
        }
        {
            int g = offer->mood[0];

            if (g < MOOD_MIN) {
                g = MOOD_MIN;
            }
            if (g > MOOD_MAX) {
                g = MOOD_MAX;
            }
            offer->mood[0] = g;
        }
        {
            int g = offer->mood[1];

            if (g < MOOD_MIN) {
                g = MOOD_MIN;
            }
            if (g > MOOD_MAX) {
                g = MOOD_MAX;
            }
            offer->mood[1] = g;
        }
        {
            int g = offer->mood[2];

            if (g < MOOD_MIN) {
                g = MOOD_MIN;
            }
            if (g > MOOD_MAX) {
                g = MOOD_MAX;
            }
            offer->mood[2] = g;
        }
        {
            int g = offer->mood[3];

            if (g < MOOD_MIN) {
                g = MOOD_MIN;
            }
            if (g > MOOD_MAX) {
                g = MOOD_MAX;
            }
            offer->mood[3] = g;
        }
    }
}
#else
INCLUDE_ASM("btlp/nonmatchings/moonmoods", BtlMoodsFromMoon);
#endif

