/* Persona 1 (JP) - an offer saying how it feels. BTLP only.
 *   0x8006EAB8 BtlOfferMoodLine
 *
 * A line keyed on the two gauges that have reached the weak level: the first
 * of the four that is there, and how far above it the next one is. The two
 * together pick a cell of a forty-entry table sitting just in front of the
 * message table BtlMessage reads, and the cell holds three lines to say one of
 * at random - the same three-to-a-cell shape the talk scripts have.
 *
 * The three inserts are filled first, the way BtlTalkAnswer fills them, so the
 * line can name the demon, the Persona on offer and its arcana. The scene is
 * pushed as 0xC - the one a two-part answer is followed by - and the offer is
 * marked so it only ever says this once.
 *
 * Nothing in the overlay calls it.
 */
#include <decomp/types.h>
#include <psyq/rand.h>
#include <persona/common/persona.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/front.h>
#include <persona/btlp/message.h>
#include <persona/btlp/offer.h>
#include <persona/btlp/talk.h>

/* Which insert each name goes in, as talkanswer.c has them. */
#define INSERT_PERSONA 2
#define INSERT_DEMON   4
#define INSERT_ARCANA  6

/* The mood lines' own table of directory slots, forty shorts ending where
   BtlMessage's table begins: four to a cell, of which three are used. */
#define BTL_MOOD_LINE_TABLE 0x364
#define BTL_MOOD_LINE_CELL  4
#define BTL_MOOD_LINE_WAYS  3

/* Where each gauge's cells start, one entry per gauge: the first gauge is
   weighed against the three above it, the last against none. */
#define BTL_MOOD_LINE_AT { 0, 0x10, 0x1C, 0x24 }

/* The weak level's four bits of `kinds`. */
#define OFFER_WEAK_FIRST 0x10

/* Set once the offer has said its line. */
#define OFFER_MOOD_SAID 0x2000000

/* What it is played as, and the scene it is pushed as. */
#define TALK_SEQ_STATE   4
#define TALK_SEQ_FRAMES  0xC
#define TALK_SCENE_AFTER 0xC
#define TALK_STAGE_OPEN  1

extern int BtlSeqState(void);

void BtlOfferMoodLine(int slot)
{
    BtlOffer *o;
    u_short  *line;
    u_long    bit;
    u_long    dir;
    int       i;
    int       above;
    u_char    at[BTL_MOODS] = BTL_MOOD_LINE_AT;

    line = (u_short *)((u_char *)g_btl_scratch_end + BTL_MOOD_LINE_TABLE);
    o    = &g_btl_offer[slot];
    BtlSetInsert(INSERT_DEMON, g_btl_actors[g_btl_actor_slot].c.name);
    BtlSetInsert(INSERT_PERSONA, g_btl_offer[g_btl_offer_slot].name);
    BtlSetInsert(INSERT_ARCANA,
                 g_btl_arcana_names[g_persona_data[g_btl_offer[g_btl_offer_slot].persona].arcana]);
    bit = OFFER_WEAK_FIRST;
    if ((o->kinds & OFFER_MOOD_SAID) != 0) {
        return;
    }

    g_btl_offer_slot = slot;
    BtlFrontSlotSet(0, 0, 0);
    i = 0;
    do {
        if ((bit & o->kinds) != 0) {
            above = 0;
            while (above < BTL_MOODS - 1 - i) {
                bit <<= 1;
                if ((bit & o->kinds) != 0) {
                    break;
                }
                above++;
            }
            line += at[i] + above * BTL_MOOD_LINE_CELL
                    + rand() % BTL_MOOD_LINE_WAYS;
            BtlSeqSetState(TALK_SEQ_STATE, TALK_SEQ_FRAMES);
            while (BtlSeqState() != 0) {
                BtlDrawFrame();
            }
            g_btl_talk_scene[g_btl_talk_depth] = TALK_SCENE_AFTER;
            g_btl_talk_stage[g_btl_talk_depth] = TALK_STAGE_OPEN;
            g_btl_talk_depth++;
            {
                /* The tail of BtlMessage, inlined: the directory offset is
                   added twice over, and the script pointer has to be the
                   thing the scratch base is added to. */
                u_char *script;

                dir    = *(u_long *)BTL_SCRATCH;
                script = *(u_long *)(BTL_SCRATCH + dir + *line * 4)
                         + BTL_SCRATCH;
                BtlSeqPlay(script + dir);
            }
            o->kinds |= OFFER_MOOD_SAID;
            return;
        }
        i++;
        bit <<= 1;
    } while (i < BTL_MOODS);
}
