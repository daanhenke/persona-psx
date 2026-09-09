/* Persona 1 (JP) - the negotiation scene that can leave the demons charmed.
 *   0x80071D84 BtlTalkSceneCharm    BTLP only.
 *
 * BtlTalkSceneStep runs this while the top of g_btl_talk_scene is 6. It is one
 * of four scenes of the same shape - the others can end in HAPPY, PANIC and
 * BIND - and the stage below the top drives it a step at a time.
 *
 * The running stage rolls for what the demons do. Four thresholds are copied
 * onto the stack and rand() % 0x100 is measured against them: an eighth of the
 * rolls say one line and go, three eighths say two and go, and the rest reach
 * the contact. Where the party is high enough for the Persona on offer and has
 * room for it, the last two thresholds are rewritten so a single roll out of
 * 256 hands it over instead.
 *
 * The contact weighs the acting member's number against the demon's: at or
 * above it the negotiation ends with every demon the offer involved charmed,
 * below it they are unmoved. Either way the offer is marked so it cannot be
 * tried this way again.
 *
 * Each outcome pops the level and takes the negotiation down; the phase it
 * leaves behind says whether the battle carries on, is over, or has been won.
 */
#include <decomp/types.h>
#include <rand.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/offer.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/text.h>

/* g_btl_talk_stage: the running stage, then one per outcome of its roll. */
#define TALK_STAGE_FREE    0
#define TALK_STAGE_RUN     1
#define TALK_STAGE_ONELINE 2
#define TALK_STAGE_TWOLINE 3
#define TALK_STAGE_TOUCH   4
#define TALK_STAGE_TAKE    5
#define TALK_STAGE_GO      6

/* g_btl_talk_scene, for a level nothing is running on, and the one a second
   line is said under. */
#define TALK_SCENE_NONE 0xFF
#define TALK_SCENE_MORE 9

/* Which lines it says. The second argument is the scene's kind. */
#define TALK_MSG_KIND  4
#define TALK_MSG_TAKE1 4
#define TALK_MSG_TAKE2 5
#define TALK_MSG_ONE   6
#define TALK_MSG_TWO   7
#define TALK_MSG_GO    8

/* Where the boxes go. */
#define TALK_TEXT_X 0x28
#define TALK_TEXT_Y 0x92
#define TALK_BOX_W  0x11
#define TALK_BOX_X  0xA0

/* The pack BtlTalkEndStatus plays as the demons take the status. */
#define TALK_PACK_CHARM 0xD

/* Which of the two offer levels the Persona is gated on here. */
#define TALK_OFFER_LEVEL 1

/* What BtlStockHasRoom answers when there is somewhere to put it. */
#define STOCK_HAS_ROOM 1

/* The outcomes the roll can reach, and how the level test opens the last. */
#define TALK_ODDS 4
#define TALK_ODDS_ALL  0x100
#define TALK_ODDS_ALMOST 0xFF

/* Set on the offer once it has been contacted this way. */
#define OFFER_CONTACTED 0x10000000

/* Bit 0 of g_btl_talk_flags: a contact has ended. */
#define TALK_FLAG_ENDED 1

/* Where the battle goes afterwards. */
#define BTL_PHASE_COMMAND 0
#define BTL_PHASE_WON     2
#define BTL_PHASE_OVER    3

extern const u_char *g_btl_talk_left_script;
extern const u_char *g_btl_talk_charm_script;
extern const u_char *g_btl_talk_unmoved_script;

/* Its slot is a u_short, which is what reads g_btl_offer_slot unsigned
   here and signed where the offer itself is indexed. */
extern int   BtlOfferLevelTest(int level, u_short slot);
extern const u_char *BtlMessage(int line, int kind);
extern void  BtlTalkEndStatus(u_char status, int pack);
extern void  BtlTalkEndStep(void);
extern void  BtlTakeOffer(u_short persona, const u_char *script);
extern void  BtlBoxOpen(short cols, short x, short y, int style);
extern void  BtlShowAilmentMarks(int show);

/* Twelve bytes of dead .rodata sit ahead of the thresholds in the original
   object - the stare scene's, whatever this file looked like before it was
   split. Nothing reads them, and the compiler builds the stare's own copy out
   of immediate stores rather than from here. They are not decoration: the jump
   table below is aligned to eight from the start of the block, so without them
   it lands four bytes early and drags the rest of the overlay's rodata with
   it. */
const int g_btl_talk_stare_odds[3] = { 0x80, TALK_ODDS_ALL, 0 };

void BtlTalkSceneCharm(void)
{
    /* Kept in .rodata and copied onto the stack on every call, which is what
       lets the level test below rewrite two of them. */
    int  odds[TALK_ODDS] = { 0x20, 0x80, TALK_ODDS_ALL, 0 };
    int  roll;
    int  i;
    int *p;

    switch (g_btl_talk_stage[g_btl_talk_depth - 1]) {
    case TALK_STAGE_RUN:
        if (BtlOfferLevelTest(TALK_OFFER_LEVEL, g_btl_offer_slot) != 0
            && BtlStockHasRoom() == STOCK_HAS_ROOM) {
            odds[2] = TALK_ODDS_ALMOST;
            odds[3] = TALK_ODDS_ALL;
        }
        roll = rand() % TALK_ODDS_ALL;
        i = 0;
        p = odds;
        while (i < TALK_ODDS) {
            if (roll < *p) {
                break;
            }
            i++;
            p++;
        }
        g_btl_talk_stage[g_btl_talk_depth - 1] = i + TALK_STAGE_ONELINE;
        break;

    case TALK_STAGE_ONELINE:
        g_btl_talk_stage[g_btl_talk_depth - 1] = TALK_STAGE_GO;
        BtlSeqRun();
        BtlSeqPlay(BtlMessage(TALK_MSG_ONE, TALK_MSG_KIND));
        BtlSeqWaitDone();
        break;

    case TALK_STAGE_TWOLINE:
        g_btl_talk_stage[g_btl_talk_depth - 1] = TALK_STAGE_GO;
        g_btl_talk_scene[g_btl_talk_depth] = TALK_SCENE_MORE;
        g_btl_talk_stage[g_btl_talk_depth] = TALK_STAGE_RUN;
        g_btl_talk_depth++;
        BtlSeqRun();
        BtlSeqPlay(BtlMessage(TALK_MSG_ONE, TALK_MSG_KIND));
        BtlSeqRun();
        BtlSeqPlay(BtlMessage(TALK_MSG_TWO, TALK_MSG_KIND));
        BtlSeqRun();
        break;

    case TALK_STAGE_TOUCH:
        if (g_btl_actors[g_btl_actor_slot].unk3A
            >= g_btl_enemies[g_btl_talk_target].unk3A) {
            BtlEndTalking();
            BtlTalkEndStatus(BTL_STATUS_CHARM, TALK_PACK_CHARM);
            BtlTextOpen(g_btl_talk_charm_script, TALK_TEXT_X, TALK_TEXT_Y);
            BtlBoxOpen(TALK_BOX_W, TALK_BOX_X, TALK_TEXT_Y, 0);
            BtlWaitAnyKey();
            BtlTalkEndStep();
        } else {
            BtlTextOpen(g_btl_talk_unmoved_script, TALK_TEXT_X, TALK_TEXT_Y);
            BtlBoxOpen(TALK_BOX_W, TALK_BOX_X, TALK_TEXT_Y, 0);
            BtlWaitAnyKey();
        }
        /* The offer is marked before the level is popped; gcc sinks the store
           into place, and writing it in its apparent position does not. */
        g_btl_offer[g_btl_offer_slot].kinds |= OFFER_CONTACTED;
        g_btl_talk_flags |= TALK_FLAG_ENDED;
        g_btl_talk_depth--;
        g_btl_talk_scene[g_btl_talk_depth] = TALK_SCENE_NONE;
        g_btl_talk_stage[g_btl_talk_depth] = TALK_STAGE_FREE;
        BtlShowAilmentMarks(1);
        BtlFaceClose();
        BtlPanelClose();
        BtlBoxClose();
        BtlSeqClear();
        BtlHudHide();
        BtlEnemiesReset();
        BtlPartyReset();
        g_btl_phase = BTL_PHASE_COMMAND;
        break;

    case TALK_STAGE_TAKE:
        g_btl_talk_depth--;
        g_btl_talk_scene[g_btl_talk_depth] = TALK_SCENE_NONE;
        g_btl_talk_stage[g_btl_talk_depth] = TALK_STAGE_FREE;
        BtlSeqRun();
        BtlSeqPlay(BtlMessage(TALK_MSG_TAKE1, TALK_MSG_KIND));
        BtlSeqRun();
        BtlTakeOffer(g_btl_offer[g_btl_offer_slot].persona,
                     BtlMessage(TALK_MSG_TAKE2, TALK_MSG_KIND));
        BtlShowAilmentMarks(1);
        BtlFaceClose();
        BtlPanelClose();
        BtlBoxClose();
        BtlSeqClear();
        BtlHudHide();
        BtlEnemiesReset();
        BtlPartyReset();
        g_btl_phase = BTL_PHASE_WON;
        break;

    case TALK_STAGE_GO:
        g_btl_talk_depth--;
        g_btl_talk_scene[g_btl_talk_depth] = TALK_SCENE_NONE;
        g_btl_talk_stage[g_btl_talk_depth] = TALK_STAGE_FREE;
        BtlSeqRun();
        BtlSeqPlay(BtlMessage(TALK_MSG_GO, TALK_MSG_KIND));
        BtlSeqWaitDone();
        BtlTalkersLeave();
        BtlTextOpen(g_btl_talk_left_script, TALK_TEXT_X, TALK_TEXT_Y);
        BtlBoxOpen(TALK_BOX_W, TALK_BOX_X, TALK_TEXT_Y, 0);
        BtlWaitAnyKey();
        BtlShowAilmentMarks(1);
        BtlFaceClose();
        BtlPanelClose();
        BtlBoxClose();
        BtlSeqClear();
        BtlHudHide();
        BtlEnemiesReset();
        BtlPartyReset();
        g_btl_phase = BTL_PHASE_OVER;
        break;
    }
}
