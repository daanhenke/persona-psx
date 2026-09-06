/* Persona 1 (JP) - showing a demon something it likes.  BTLP only.
 *   0x800744C8 BtlTalkLikedEquip
 *
 * Eight entries pair a piece of equipment with the four species that think
 * well of whoever carries it. If the speaker has that piece in any of their
 * seven slots and the demon being spoken to is one of the four, the mood
 * currently being worked on goes straight to its maximum.
 *
 * Moods 1 and 2 are the exception: a match on either is reported as no match
 * and nothing is raised, so those two can only be moved the ordinary way.
 */
#include <types.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/offer.h>

#define LIKED_ENTRIES 8
#define LIKED_SPECIES 4
#define CHAR_EQUIP    7

typedef struct {
    /* 0x0 */ u_long item;
    /* 0x4 */ u_char pad04[4];
    /* 0x8 */ u_char likes[LIKED_SPECIES];
} BtlLikedEquip;                    /* 0xC bytes */

extern const BtlLikedEquip g_btl_liked_equip[];
extern BtlActor g_btl_actors[];
extern BtlActor g_btl_enemies[];
extern short    g_btl_actor_slot;
extern short    g_btl_offer_slot;
extern short    g_btl_talk_target;
extern short    g_btl_talk_said;

int BtlTalkLikedEquip(void)
{
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
    do {
        equip = g_btl_actors[g_btl_actor_slot].c.equip;
        i = 0;
        do {
            if ((u_long)*equip ==
                *(const u_long *)((const char *)g_btl_liked_equip + off)) {
                end = like;
                do {
                    species = *end;
                    end++;
                    if (species == g_btl_enemies[g_btl_talk_target].species) {
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
    if (found) {
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
