/* Persona 1 (JP) - what the demon says back.  BTLP only.
 *   0x8006E56C BtlTalkAnswer
 *
 * A round of negotiation ends with one talk act, and the demon answers the
 * *pair* the round amounts to rather than that act on its own. The partner
 * comes from the moon: at a new or full moon it is the first of the four moods
 * in a fixed order that the demon is already strongly in, and at every other
 * phase it is whatever act was used most recently before this one.
 *
 * BtlTalkPairIndex turns the two-bit mask into one of ten rows, and the row
 * says which scene the answer is played as and which message it says. Rows
 * that ask for it push a second scene, 0xC, on top of the first. The offer's
 * low nibble is left holding the acts the answer was based on.
 *
 * The three inserts are filled first so the message can name the demon, the
 * Persona on offer and its arcana.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <persona/common/persona.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/offer.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/talk.h>

/* Which insert each name goes in. */
#define INSERT_PERSONA 2
#define INSERT_DEMON   4
#define INSERT_ARCANA  6

/* The two phases that pick the partner from a fixed order. */
#define MOON_NEW  0
#define MOON_FULL 8

/* Talk acts, and therefore bits of the mask and rows of the mood order. */
#define BTL_TALK_ACTS 4

/* Nothing recent enough to pair with. */
#define BTL_RECENT_NONE (-1)

/* The panel that shows which acts the round used. */
#define PANEL_ACTS 2

/* What the answer is played as: state 4 for four frames, and the extra scene a
   two-part answer is followed by. */
#define TALK_SEQ_STATE  4
#define TALK_SEQ_FRAMES 4
#define TALK_SCENE_AFTER 0xC
#define TALK_STAGE_OPEN  1

/* The acts live in the low nibble of the offer's mask. */
#define TALK_ACTS_MASK 0xF

/* One row per unordered pair of acts. */
typedef struct {
    /* 0x0 */ short  two_part; /* the answer is followed by scene 0xC */
    /* 0x2 */ u_char scene;    /* the talk scene the answer is played as */
    /* 0x3 */ u_char pad03[1];
    /* 0x4 */ short  group;    /* the message it says */
    /* 0x6 */ short  index;
} BtlTalkAnswerRow;            /* 8 bytes */

extern const BtlTalkAnswerRow g_btl_talk_answers[];
/* The pair index expanded back into the four act bits. */
extern const short  g_btl_talk_pair_acts[];
/* The order the moods are tried in, one table per phase that uses one. */
extern const u_long g_btl_moon_new_partners[];
extern const u_long g_btl_moon_full_partners[];

extern const u_char *g_btl_arcana_names[];
extern u_char    g_btl_moon;

extern void          BtlSetInsert(int slot, const u_char *text);
extern void          BtlPanelSetImage(int group, u_char image);
extern int           BtlTalkPairIndex(u_int acts);
extern void          BtlSeqSetState(int state, int frames);
extern const u_char *BtlMessage(int group, int index);
extern int           BtlRecentOther(int value);

#ifdef NON_MATCHING
void BtlTalkAnswer(int slot, u_int act)
{
    BtlOffer *o;
    u_int     acts;
    const u_long *p;
    int       one;
    int       i;
    int       pair;

    o = g_btl_offer + slot;
    BtlSetInsert(INSERT_DEMON, g_btl_actors[g_btl_actor_slot].c.name);
    BtlSetInsert(INSERT_PERSONA, g_btl_offer[g_btl_offer_slot].name);
    BtlSetInsert(INSERT_ARCANA,
                 g_btl_arcana_names[g_persona_data[
                     g_btl_offer[g_btl_offer_slot].persona].arcana]);

    acts = 1 << act;
    if (g_btl_moon == MOON_NEW) {
        i = 0;
        one = 1;
        p = g_btl_moon_new_partners;
        /* A plain `while`: written as a do/while gcc peels the first test,
           which the original does not. */
        while (i < BTL_TALK_ACTS) {
            if (act != *p && (one << *p & o->kinds) != 0) {
                break;
            }
            i++;
            p++;
        }
        if (i != BTL_TALK_ACTS) {
            acts |= 1 << g_btl_moon_new_partners[i];
        }
    }
    i = 0;
    if (g_btl_moon == MOON_FULL) {
        one = 1;
        p = g_btl_moon_full_partners;
        /* A plain `while`: written as a do/while gcc peels the first test,
           which the original does not. */
        while (i < BTL_TALK_ACTS) {
            if (act != *p && (one << *p & o->kinds) != 0) {
                break;
            }
            i++;
            p++;
        }
        if (i != BTL_TALK_ACTS) {
            acts |= 1 << g_btl_moon_full_partners[i];
        }
    }
    if (g_btl_moon != MOON_NEW && g_btl_moon != MOON_FULL) {
        i = BtlRecentOther(act);
        if (i != BTL_RECENT_NONE) {
            acts |= 1 << i;
        }
    }

    BtlPanelSetImage(PANEL_ACTS, acts);
    pair = BtlTalkPairIndex(acts);
    g_btl_talk_pair = pair;
    BtlSeqSetState(TALK_SEQ_STATE, TALK_SEQ_FRAMES);
    BtlSeqWaitDone();

    g_btl_talk_scene[g_btl_talk_depth] = g_btl_talk_answers[pair].scene;
    g_btl_talk_stage[g_btl_talk_depth] = TALK_STAGE_OPEN;
    g_btl_talk_depth++;
    g_btl_offer[g_btl_offer_slot].kinds &= ~TALK_ACTS_MASK;
    g_btl_offer[g_btl_offer_slot].kinds |= g_btl_talk_pair_acts[pair];

    if (g_btl_talk_answers[pair].two_part == 0) {
        BtlSeqPlay(BtlMessage(g_btl_talk_answers[pair].group,
                              g_btl_talk_answers[pair].index));
        BtlSeqRun();
    } else {
        /* The two stores in a block of their own. Without the boundary gcc
           schedules the message's group load to the top of the arm instead of
           after them, which is load-bearing rather than decoration. */
        do {
            g_btl_talk_scene[g_btl_talk_depth] = TALK_SCENE_AFTER;
            g_btl_talk_stage[g_btl_talk_depth] = TALK_STAGE_OPEN;
        } while (0);
        g_btl_talk_depth++;
        BtlSeqPlay(BtlMessage(g_btl_talk_answers[pair].group,
                              g_btl_talk_answers[pair].index));
    }
}
#else
INCLUDE_ASM("btlp/nonmatchings/talkanswer", BtlTalkAnswer);
#endif

