/* Persona 1 (JP) - sorting the enemies into offers.  BTLP only.
 *   0x800697F8 BtlBuildOffers
 *
 * Run as the battle opens and again whenever the negotiation goes back to
 * looking for someone to talk to. It throws the three offer records away and
 * builds them from whatever is still on the field.
 *
 * The party half comes first: each member's key is copied into g_btl_member
 * along with the two bytes the Persona lookup is run against, and the levels
 * of the occupied slots are averaged into g_btl_talk_level. That average is
 * what every level comparison in the negotiation is made against.
 *
 * Then the enemies, one slot at a time. An enemy carrying a Persona joins the
 * offer that already holds that Persona, or takes a free one and fills it in;
 * either way its bit goes into `used`, its own ailment is recorded against its
 * slot, and its health is added to the offer's totals. An offer being joined
 * for the first time this round moves last round's health total aside first,
 * so the difference is what the round has cost the demons.
 *
 * Finally the offers are counted: one that ended up with no demons has its
 * `used` cleared, and the rest are marked in use.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <decomp/libc.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/member.h>
#include <persona/btlp/offer.h>
#include <persona/btlp/talk.h>

/* Bytes of the demon's name the offer keeps. */
#define OFFER_NAME_BYTES 10

/* Set on an offer that holds anybody at all. */
#define OFFER_IN_USE 0x80000000

/* The demon's own row: which voice it speaks with, the flags the negotiation
   reads it through, and where its four gauges start. */
extern const u_char *g_btl_demon_talk_profiles[];

#define PROFILE_VOICE 0
#define PROFILE_FLAGS 2
#define PROFILE_MOOD  4
#define PROFILE_MOOD3 7

extern void BtlSetMemberAnswers(void);
extern void BtlOfferPickTalkers(void);
extern int  BtlOfferFind(int species);
extern int  BtlOfferFree(void);

#ifdef NON_MATCHING
void BtlBuildOffers(void)
{
    const u_char *profile;
    BtlActor     *a;
    BtlActor     *e;
    BtlMember    *m;
    BtlOffer     *offer;
    BtlOffer     *rec;
    int           counted;
    int           slot;
    int           held;
    int           was;
    int           i;

    /* The enemies are the tail of the same table, and reaching them that way
       rather than by their own symbol is what lets one base address serve
       both walks. */
    a = g_btl_actors;
    e = a + BTL_PARTY;
    m = g_btl_member;
    counted = 0;
    i = 0;
    g_btl_talk_level = 0;
    do {
        m->key = a->c.key;
        if (a->c.key != 0) {
            g_btl_talk_level += a->c.level;
            m->pair[0] = a->c.status;
            m->pair[1] = a->c.ail_level;
            counted++;
        }
        a++;
        i++;
        m++;
    } while (i < BTL_PARTY);
    g_btl_talk_level = g_btl_talk_level / counted;

    i = 0;
    rec = g_btl_offer;
    do {
        rec->demons = 0;
        rec->used = 0;
        rec++;
        i++;
    } while (i < BTL_OFFERS);
    g_btl_talker_count = 0;

    i = 0;
    do {
        if (e->c.key != 0) {
            slot = BtlOfferFind(e->c.key);
            if (slot == BTL_NO_OFFER) {
                offer = &g_btl_offer[BtlOfferFree()];
                offer->used = 1 << i;
                offer->persona = e->c.key;
                offer->species = e->species;
                offer->level = e->c.level;
                offer->status[i] = e->c.status;
                offer->ail_level[i] = e->c.ail_level;
                offer->hp = e->c.hp_max;
                memcpy(offer->name, e->c.name, OFFER_NAME_BYTES);
                profile = g_btl_demon_talk_profiles[e->c.key];
                offer->voice = profile[PROFILE_VOICE];
                offer->flags = *(const u_short *)&profile[PROFILE_FLAGS];
                offer->mood[0] = profile[PROFILE_MOOD];
                offer->mood[1] = profile[PROFILE_MOOD + 1];
                offer->mood[2] = profile[PROFILE_MOOD + 2];
                held = profile[PROFILE_MOOD3];
                offer->kinds = 0;
                offer->demons++;
                offer->mood[3] = held;
            } else {
                offer = &g_btl_offer[slot];
                if (offer->demons == 0) {
                    was = offer->hp_now;
                    offer->hp = 0;
                    offer->hp_now = 0;
                    offer->hp_was = was;
                }
                offer->used |= 1 << i;
                offer->status[i] = e->c.status;
                offer->ail_level[i] = e->c.ail_level;
                offer->hp += e->c.hp_max;
                offer->hp_now += e->c.hp;
                offer->demons++;
            }
            g_btl_talker_count++;
        }
        e++;
        i++;
    } while (i < BTL_ENEMIES);

    g_btl_offer_count = 0;
    rec = g_btl_offer;
    i = 0;
    do {
        if (rec->demons != 0) {
            if ((rec->kinds & OFFER_IN_USE) == 0) {
                rec->kinds |= OFFER_IN_USE;
            }
            g_btl_offer_count++;
        } else {
            rec->used = 0;
        }
        rec++;
        i++;
    } while (i < BTL_OFFERS);

    BtlSetMemberAnswers();
    BtlOfferPickTalkers();
    BtlMarkMembersMatched();
    BtlMarkOffersLive();
}
#else
INCLUDE_ASM("btlp/nonmatchings/buildoffers", BtlBuildOffers);
#endif
