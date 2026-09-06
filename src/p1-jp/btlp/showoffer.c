/* Persona 1 (JP) - pointing the panel at the offer in hand.  BTLP only.
 *   0x8006C6B4 BtlPanelShowOffer
 *
 * Called every frame the offer is on screen, but the picture is only rebuilt
 * when the offer has actually changed - the remembered slot is what makes the
 * rest of the frames free.
 *
 * The picture comes out of the low nibble of the offer's mask, which is the
 * same mask BtlOfferMarkStrong keeps: which of the four gauges have reached the
 * higher of the two levels.
 */
#include <types.h>
#include <persona/btlp/offer.h>

/* The group the offers' pictures live in. */
#define PANEL_GROUP_OFFER 2

extern int   g_btl_panel_offer;
extern short g_btl_offer_slot;

extern void BtlPanelSetImage(u_char group, u_char index);

void BtlPanelShowOffer(void)
{
    u_long kinds;

    /* The mask is fetched before the test, not inside it; the slot itself is
       read straight from the global each time, and gcc re-reads it after the
       call rather than holding it. */
    kinds = g_btl_offer[g_btl_offer_slot].kinds;
    if (g_btl_panel_offer != g_btl_offer_slot) {
        BtlPanelSetImage(PANEL_GROUP_OFFER, kinds & 0xF);
        g_btl_panel_offer = g_btl_offer_slot;
    }
}
