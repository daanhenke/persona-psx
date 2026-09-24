/* Persona 1 (JP) - showing a demon something it likes.  BTLP only.
 *   0x800744C8 BtlTalkLikedEquip  0x80074614 BtlTalkGiveItem
 *
 * Eight entries pair a piece of equipment with the four species that think
 * well of whoever carries it. If the speaker has that piece in any of their
 * seven slots and the demon being spoken to is one of the four, the mood
 * currently being worked on goes straight to its maximum.
 *
 * Moods 1 and 2 are the exception: a match on either is reported as no match
 * and nothing is raised, so those two can only be moved the ordinary way.
 *
 * BtlTalkGiveItem is the other half of the same table: where the routine above
 * scores a mood for carrying the right thing, this one has the demon hand a
 * piece over. It reads the record the other way round - the demon's species
 * picks the record and the speaker has to be one of the four characters named
 * at +4 - and the gift still needs a coin flip, a talk level of at least 30,
 * and room in the party's bags.
 */
#include <decomp/types.h>
#include <rand.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/offer.h>
#include <persona/common/item.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/talk.h>

#define LIKED_ENTRIES 8
#define LIKED_SPECIES 4
#define CHAR_EQUIP    7

typedef struct {
    /* 0x0 */ u_long item;
    /* 0x4 */ u_char givento[LIKED_SPECIES];
                                    /* Char.key of the characters this may be
                                       handed to; BtlTalkGiveItem refuses any
                                       other speaker                        */
    /* 0x8 */ u_char likes[LIKED_SPECIES];
} BtlLikedEquip;                    /* 0xC bytes */

extern const BtlLikedEquip g_btl_liked_equip[];


/* Below this the demon keeps its things whatever it thinks of the speaker. */
#define GIFT_LEVEL 0x1E

/* Which slot of the message template the item's name is dropped into. */
#define INSERT_ITEM 3

extern int          BtlOfferLevelTest(int test, u_short slot);
extern void         BtlSetInsert(int kind, const u_char *text);

/* The equipment is walked with a pointer that starts at the array, and the
   same pointer is then pointed at the offer's moods and stepped to the one
   being answered. n is declared ahead of the second record: the two tie on
   global alloc's priority, and a tie goes to the lower-numbered pseudo. */
int BtlTalkLikedEquip(void)
{
    BtlActor *me;
    int       n;
    BtlActor *him;
    int       found;
    int       i;
    int       k;
    u_short  *eq;

    found = 0;
    me = &g_btl_actors[g_btl_actor_slot];
    him = &g_btl_actors[BTL_PARTY + g_btl_talk_target];
    for (n = 0; n < LIKED_ENTRIES; n++) {
        eq = me->c.equip;
        for (i = 0; i < CHAR_EQUIP; i++, eq++) {
            if (*eq == g_btl_liked_equip[n].item) {
                for (k = 0; k < LIKED_SPECIES; k++) {
                    if (g_btl_liked_equip[n].likes[k] == him->species) {
                        found = 1;
                        goto done;
                    }
                }
            }
        }
    }
done:
    if (found == 1) {
        if ((u_short)(g_btl_talk_said - 1) < 2) {
            return 0;
        }
        eq = (u_short *)g_btl_offer[g_btl_offer_slot].mood;
        eq += g_btl_talk_said;
        *eq = BTL_MOOD_STRONG;
        return 1;
    }
    return 0;
}

/* The enemy is reached as a slot of g_btl_actors rather than through
   g_btl_enemies: the original builds one base and adds the five party records
   to it, which is the same table said the other way. */
int BtlTalkGiveItem(void)
{
    const BtlActor *me;
    const BtlActor *him;
    int             n;
    int             i;
    u_long          item;

    me = &g_btl_actors[g_btl_actor_slot];
    him = &g_btl_actors[BTL_PARTY + g_btl_talk_target];
    if (BtlOfferLevelTest(0, g_btl_offer_slot) != 1) {
        return 0;
    }
    if ((rand() & 1) != 0) {
        for (n = 0; n < LIKED_ENTRIES; n++) {
            for (i = 0; i < LIKED_SPECIES; i++) {
                if (g_btl_liked_equip[n].likes[i] == him->species) {
                    break;
                }
            }
            if (i != LIKED_SPECIES) {
                break;
            }
        }
        if (n != LIKED_ENTRIES) {
            item = g_btl_liked_equip[n].item;
            for (i = 0; i < LIKED_SPECIES; i++) {
                if (me->c.key == g_btl_liked_equip[n].givento[i]) {
                    break;
                }
            }
            if (i == LIKED_SPECIES) {
                return 0;
            }
            if (g_btl_talk_level < GIFT_LEVEL) {
                return 0;
            }
            if ((u_short)BtlItemSlot(item & 0xFFFF) != 0) {
                BtlSetInsert(INSERT_ITEM, g_item_defs[item].name);
                BtlSeqPlay(g_btl_talk_gift_script);
                BtlSeqWaitDone();
                BtlItemAdd((short)item);
                return 1;
            }
        }
    }
    return 0;
}

