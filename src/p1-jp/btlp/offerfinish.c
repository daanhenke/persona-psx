/* Persona 1 (JP) - closing out an offer.  BTLP only.
 *   0x8007010C BtlOfferFinish
 *
 * Two things, in one breath. Every enemy the offer involved is marked, and
 * nothing in the battle reads that mark back - it is left for whatever runs
 * after the fight. Then the music is swapped for pack entry 0x10 and played
 * straight, with no fade either side, which is what makes it read as a jingle
 * rather than a change of background music. All three callers hold three
 * seconds afterwards to let it play.
 */
#include <types.h>
#include <libsnd.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/offer.h>
#include <persona/btlp/sound.h>

/* The pack entry the jingle lives in. */
#define BTL_OFFER_PACK 0x10

extern BtlActor      g_btl_enemies[];
extern short         g_btl_offer_slot;
extern volatile long g_cd_busy;

extern void BtlLoadPackEntry(int entry);
extern void BtlBgmOpen(void);
extern void BtlDrawFrame(void);

void BtlOfferFinish(void)
{
    BtlOffer *offer;
    BtlActor *a;
    int       i;

    a = g_btl_enemies;
    i = 0;
    offer = &g_btl_offer[g_btl_offer_slot];
    do {
        if ((offer->used >> i & 1) != 0) {
            a->offered = 1;
        }
        i++;
        a++;
    } while (i < BTL_ENEMIES);

    BtlLoadPackEntry(BTL_OFFER_PACK);
    while (g_cd_busy != -1) {
        BtlDrawFrame();
    }
    BtlBgmOpen();
    SsVabTransCompleted(1);
    BtlSePlay(BTL_BGM_SLOT, 0);
}
