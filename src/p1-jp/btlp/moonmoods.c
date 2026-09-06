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
#include <types.h>
#include <persona/btlp/offer.h>

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

extern u_char g_btl_moon;
extern int    g_btl_level_gap;

extern int BtlOfferLevelTest(int test, u_short slot);
extern int BtlStockHolds(const BtlOffer *offer);

void BtlMoodsFromMoon(void)
{
    BtlOffer *offer;
    int       step;
    short     v;
    u_int     slot;

    offer = g_btl_offer;
    slot = 0;
    do {
        if ((offer->flags & OFFER_SHOWN) == 0 || offer->used == 0) {
            goto clamp;
        }
        switch (g_btl_moon) {
        case MOON_NEW:
            offer->mood[0] += 10;
            break;
        case MOON_CRESCENT:
        case MOON_WANING:
            v = offer->mood[2] + 5;
            goto set;
        case MOON_HALF:
        case MOON_GIBBOUS:
            offer->mood[1] += 5;
            break;
        case MOON_FULL:
            offer->mood[1] += 10;
            offer->mood[3] += 10;
            v = offer->mood[2] + 10;
        set:
            offer->mood[2] = v;
        }

        if (BtlOfferLevelTest(3, slot) != 0) {
            step = g_btl_level_gap / 10;
            offer->mood[2] += step * -MOOD_PER_LEVEL;
            offer->mood[1] += step * MOOD_PER_LEVEL;
        }
        if (BtlOfferLevelTest(4, slot) != 0) {
            step = g_btl_level_gap / 10;
            v = step * MOOD_PER_LEVEL;
            offer->mood[1] += v;
            offer->mood[2] += step * -MOOD_PER_LEVEL;
            offer->mood[0] += v;
        }
        if (BtlStockHolds(offer) != 0) {
            offer->mood[0] += MOOD_HELD;
            offer->mood[3] += MOOD_HELD;
        }
    clamp:
        v = offer->mood[0];
        step = v;
        if (v < MOOD_MIN) {
            step = MOOD_MIN;
        }
        if (v > MOOD_MAX) {
            step = MOOD_MAX;
        }
        v = offer->mood[1];
        offer->mood[0] = step;
        if (v < MOOD_MIN) {
            v = MOOD_MIN;
        }
        if (v > MOOD_MAX) {
            v = MOOD_MAX;
        }
        step = offer->mood[2];
        offer->mood[1] = v;
        if (step < MOOD_MIN) {
            step = MOOD_MIN;
        }
        if (step > MOOD_MAX) {
            step = MOOD_MAX;
        }
        v = offer->mood[3];
        offer->mood[2] = step;
        if (v < MOOD_MIN) {
            v = MOOD_MIN;
        }
        if (v > MOOD_MAX) {
            v = MOOD_MAX;
        }
        offer->mood[3] = v;
        slot++;
        offer++;
    } while (slot < BTL_OFFERS);
}
