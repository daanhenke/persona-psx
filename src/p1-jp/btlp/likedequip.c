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
#include <types.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/offer.h>
#include <persona/common/item.h>

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
extern BtlActor g_btl_actors[];
extern BtlActor g_btl_enemies[];
extern short    g_btl_actor_slot;
extern short    g_btl_offer_slot;
extern short    g_btl_talk_target;
extern short    g_btl_talk_said;
extern u_short  g_btl_talk_level;

extern const u_char *g_btl_talk_gift_script;

/* Below this the demon keeps its things whatever it thinks of the speaker. */
#define GIFT_LEVEL 0x1E

/* Which slot of the message template the item's name is dropped into. */
#define INSERT_ITEM 3

extern int          rand(void);
extern int          BtlOfferLevelTest(int test, u_short slot);
extern unsigned int BtlItemSlot(u_short id);
extern int          BtlItemAdd(int id);
extern void         BtlSetInsert(int kind, const u_char *text);
extern void         BtlSeqPlay(const u_char *script);
extern void         BtlSeqWaitDone(void);

int BtlTalkLikedEquip(void)
{
    BtlActor   *me;
    BtlActor   *him;
    const char *like;
    const char *end;
    u_short    *equip;
    char        species;
    int         found;
    int         result;
    int         i;
    int         n;
    int         off;

    found = 0;
    n = 0;
    like = (const char *)g_btl_liked_equip[0].likes;
    off = 0;
    me = &g_btl_actors[g_btl_actor_slot];
    him = &g_btl_actors[BTL_PARTY + g_btl_talk_target];
    do {
        equip = me->c.equip;
        i = 0;
        do {
            if ((u_long)*equip ==
                *(const u_long *)((const char *)g_btl_liked_equip + off)) {
                end = like;
                do {
                    species = *end;
                    end++;
                    if (species == him->species) {
                        found = 1;
                        goto done;
                    }
                } while (end < like + LIKED_SPECIES);
            }
            i++;
            equip++;
        } while (i < CHAR_EQUIP);
        like += sizeof(BtlLikedEquip);
        n++;
        off += sizeof(BtlLikedEquip);
    } while (n < LIKED_ENTRIES);
done:
    result = 0;
    if (found == 1) {
        result = 1;
        if ((u_short)(g_btl_talk_said - 1) < 2) {
            result = 0;
        } else {
            g_btl_offer[g_btl_offer_slot].mood[g_btl_talk_said] =
                BTL_MOOD_STRONG;
        }
    }
    return result;
}

/* The enemy is reached as a slot of g_btl_actors rather than through
   g_btl_enemies: the original builds one base and adds the five party records
   to it, which is the same table said the other way. */
int BtlTalkGiveItem(void)
{
    const BtlActor *me;
    const BtlActor *him;
    const char     *like;
    const char     *end;
    const char     *who;
    int             species;
    int             n;
    int             i;
    u_long          item;

    me = &g_btl_actors[g_btl_actor_slot];
    him = &g_btl_actors[BTL_PARTY + g_btl_talk_target];
    if (BtlOfferLevelTest(0, g_btl_offer_slot) != 1) {
        return 0;
    }
    if ((rand() & 1) != 0) {
        n = 0;
        species = him->species;
        like = (const char *)g_btl_liked_equip[0].likes;
        do {
            i = 0;
            end = like;
            do {
                if (*end == species) {
                    break;
                }
                i++;
                end++;
            } while (i < LIKED_SPECIES);
            if (i != LIKED_SPECIES) {
                break;
            }
            n++;
            like += sizeof(BtlLikedEquip);
        } while (n < LIKED_ENTRIES);
        i = 0;
        if (n != LIKED_ENTRIES) {
            who = (const char *)g_btl_liked_equip[n].givento;
            item = g_btl_liked_equip[n].item;
            do {
                if (me->c.key == *who) {
                    break;
                }
                i++;
                who++;
            } while (i < LIKED_SPECIES);
            if (i == LIKED_SPECIES) {
                return 0;
            }
            if (g_btl_talk_level < GIFT_LEVEL) {
                return 0;
            }
            if (BtlItemSlot(item & 0xFFFF) != 0) {
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
