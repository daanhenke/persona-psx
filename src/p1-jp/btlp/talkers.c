/* Persona 1 (JP) - who is left to negotiate with.  BTLP only.
 *   0x80073DEC BtlOfferPickTalkers  0x80073CC0 BtlMarkOffersLive
 *
 * An offer is made to a set of enemies, kept as a bitmask over the nine enemy
 * slots. Anything with an ailment is in no state to answer, so the set is
 * narrowed to those whose Char.status is still zero, and that narrowed set is
 * what the negotiation works from.
 *
 * Both are rebuilt from scratch each time rather than kept up to date, which
 * is why they run together at the top of a round.
 */
#include <types.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/member.h>
#include <persona/btlp/offer.h>

/* The enemy slots start after the party's five. */
extern BtlActor g_btl_enemies[];

/* One bit per offer, set while that offer still has someone able to answer,
   and one per party member the lookup answered for. */
extern u_char g_btl_offer_live;
extern u_char g_btl_member_matched;

void BtlOfferPickTalkers(void)
{
    BtlOffer *offer;
    BtlActor *enemies;
    BtlActor *actor;
    u_char    held[2];
    u_short   talkers;
    int       i;
    int       slot;

    offer = g_btl_offer;
    i = 0;
    enemies = g_btl_enemies;
    do {
        offer->talkers = 0;
        slot = 0;
        actor = enemies;
        do {
            if ((offer->used >> slot & 1) != 0) {
                /* Both bytes are read and nothing looks at them again. They
                   are in the original, so leave them where they are. */
                held[0] = offer->pad07[slot];
                held[1] = offer->pad10[slot];
                talkers = offer->talkers;
                if (*(signed char *)&actor->c.status == 0) {
                    talkers |= 1 << slot;
                }
                offer->talkers = talkers;
            }
            slot++;
            actor++;
        } while (slot < BTL_ENEMIES);
        offer++;
        i++;
    } while (i < BTL_OFFERS);
}

void BtlMarkOffersLive(void)
{
    int i;

    g_btl_offer_live = 0;
    i = 0;
    do {
        if (g_btl_offer[i].talkers != 0) {
            g_btl_offer_live |= 1 << i;
        }
        i++;
    } while (i < BTL_OFFERS);
}

/* The shifted value is the one just compared, not a fresh 1 - they are the
   same number and the original only materialises it once. */
void BtlMarkMembersMatched(void)
{
    int i;
    int answer;

    g_btl_member_matched = 0;
    i = 0;
    do {
        answer = g_btl_member[i].answer;
        if (answer == 1) {
            g_btl_member_matched |= answer << i;
        }
        i++;
    } while (i < BTL_PARTY);
}
