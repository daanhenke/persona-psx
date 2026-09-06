/* Persona 1 (JP) - taking a finished mood gauge away.  BTLP only.
 *   0x8006C718 BtlMoodRetire
 *
 * A gauge that has run its course sits on state 4 and waits here. Two of the
 * offer's flag bits belong to each gauge - four disjoint pairs covering bits 0
 * to 7 - and if either is set the gauge is held on state 5 instead of being
 * taken away.
 *
 * Clearing one away is four things at once: the offer stops counting it, its
 * mood is parked at one under the strong threshold so nothing reads it as
 * having got there, the panel stops showing it, and it comes out of the recent
 * list. The panel and all four gauges are redrawn once at the end rather than
 * per gauge.
 */
#include <types.h>
#include <persona/btlp/offer.h>

/* A gauge has finished and is waiting, or has finished and is being held. */
#define MOOD_DONE 4
#define MOOD_HELD 5

/* Where a retired gauge's mood is parked: one under BTL_MOOD_STRONG. */
#define MOOD_RETIRED 0x5E

extern const u_short g_btl_mood_flag_bits[];
extern short         g_btl_mood_state[];
extern short         g_btl_panel_gauges;
extern short         g_btl_offer_slot;

extern void BtlDropRecent(int gauge);
extern void BtlPanelSetImage(int on, u_char image);
extern void BtlSetMoodGauges(short a, short b, short c, short d);

void BtlMoodRetire(void)
{
    int i;

    i = 0;
    do {
        if (g_btl_mood_state[i] == MOOD_DONE) {
            /* Load-bearing: it puts the table's address where the original
               keeps it. Do not delete. */
            if (g_btl_offer == 0) {
            }
            if ((g_btl_offer[g_btl_offer_slot].flags &
                 g_btl_mood_flag_bits[i]) != 0) {
                g_btl_mood_state[i] = MOOD_HELD;
            } else {
                g_btl_offer[g_btl_offer_slot].kinds &= ~(1 << i);
                g_btl_offer[g_btl_offer_slot].mood[i] = MOOD_RETIRED;
                g_btl_mood_state[i] = 0;
                g_btl_panel_gauges &= ~(1 << i);
                BtlDropRecent(i);
            }
        }
        i++;
    } while (i < BTL_MOODS);

    BtlPanelSetImage(g_btl_panel_gauges != 0, g_btl_panel_gauges);
    BtlSetMoodGauges(g_btl_offer[g_btl_offer_slot].mood[0],
                     g_btl_offer[g_btl_offer_slot].mood[1],
                     g_btl_offer[g_btl_offer_slot].mood[2],
                     g_btl_offer[g_btl_offer_slot].mood[3]);
}
