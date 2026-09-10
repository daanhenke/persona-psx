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
#include <persona/common/persona.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/offer.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/talk.h>
#include <persona/btlp/panel.h>
#include <persona/btlp/message.h>

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
#define TALK_SEQ_STATE   4
#define TALK_SEQ_FRAMES  4
#define TALK_SCENE_AFTER 0xC
#define TALK_STAGE_OPEN  1

/* The acts live in the low nibble of the offer's mask. */
#define TALK_ACTS_MASK 0xF

/* These four adjacent tables are writable data in the original image. */
u_long g_btl_moon_new_partners[BTL_TALK_ACTS]  = { 0, 2, 3, 1 };
u_long g_btl_moon_full_partners[BTL_TALK_ACTS] = { 1, 3, 2, 0 };

BtlTalkAnswerRow g_btl_talk_answers[10] = {
    { 0, 1, { 0 }, 3, 0 },
    { 0, 1, { 0 }, 0, 0 },
    { 0, 1, { 0 }, 1, 0 },
    { 0, 1, { 0 }, 2, 0 },
    { 1, 3, { 0 }, 0, 3 },
    { 1, 7, { 0 }, 0, 1 },
    { 1, 2, { 0 }, 0, 2 },
    { 1, 5, { 0 }, 0, 5 },
    { 1, 6, { 0 }, 0, 4 },
    { 1, 4, { 0 }, 0, 6 }
};

short g_btl_talk_pair_acts[10] = { 1, 3, 5, 9, 2, 6, 10, 4, 12, 8 };

void BtlTalkAnswer(int slot, u_int act)
{
    BtlOffer*     o;
    u_int         acts;
    const u_long* p;
    int           one;
    int           i;
    int           pair;

    o = g_btl_offer + slot;
    BtlSetInsert(INSERT_DEMON, g_btl_actors[g_btl_actor_slot].c.name);
    BtlSetInsert(INSERT_PERSONA, g_btl_offer[g_btl_offer_slot].name);
    BtlSetInsert(INSERT_ARCANA,
                 g_btl_arcana_names[g_persona_data[g_btl_offer[g_btl_offer_slot].persona].arcana]);

    acts = 1 << act;
    if (g_btl_moon == MOON_NEW)
    {
        i   = 0;
        one = 1;
        p   = g_btl_moon_new_partners;
        /* A plain `while`: written as a do/while gcc peels the first test,
           which the original does not. */
        while (i < BTL_TALK_ACTS)
        {
            if (act != *p && (one << *p & o->kinds) != 0)
            {
                break;
            }
            i++;
            p++;
        }
        if (i != BTL_TALK_ACTS)
        {
            acts |= 1 << g_btl_moon_new_partners[i];
        }
    }
    if (g_btl_moon == MOON_FULL)
    {
        i   = 0;
        one = 1;
        p   = g_btl_moon_full_partners;
        /* A plain `while`: written as a do/while gcc peels the first test,
           which the original does not. */
        while (i < BTL_TALK_ACTS)
        {
            if (act != *p && (one << *p & o->kinds) != 0)
            {
                break;
            }
            i++;
            p++;
        }
        if (i != BTL_TALK_ACTS)
        {
            acts |= 1 << g_btl_moon_full_partners[i];
        }
    }
    if (g_btl_moon != MOON_NEW && g_btl_moon != MOON_FULL)
    {
        i = BtlRecentOther(act);
        if (i != BTL_RECENT_NONE)
        {
            acts |= 1 << i;
        }
    }

    BtlPanelSetImage(PANEL_ACTS, acts);
    pair            = BtlTalkPairIndex(acts);
    g_btl_talk_pair = pair;
    BtlSeqSetState(TALK_SEQ_STATE, TALK_SEQ_FRAMES);
    BtlSeqWaitDone();

    g_btl_talk_scene[g_btl_talk_depth] = g_btl_talk_answers[pair].scene;
    g_btl_talk_stage[g_btl_talk_depth] = TALK_STAGE_OPEN;
    g_btl_talk_depth++;
    g_btl_offer[g_btl_offer_slot].kinds &= ~TALK_ACTS_MASK;
    g_btl_offer[g_btl_offer_slot].kinds |= g_btl_talk_pair_acts[pair];

    if (g_btl_talk_answers[pair].two_part == 0)
    {
        BtlSeqPlay(BtlMessage(g_btl_talk_answers[pair].group,
                              g_btl_talk_answers[pair].index));
        BtlSeqRun();
    }
    else
    {
        g_btl_talk_scene[g_btl_talk_depth] = TALK_SCENE_AFTER;
        g_btl_talk_stage[g_btl_talk_depth] = TALK_STAGE_OPEN;
        g_btl_talk_depth++;
        BtlSeqPlay(BtlMessage(g_btl_talk_answers[pair].group,
                              g_btl_talk_answers[pair].index));
    }
}
