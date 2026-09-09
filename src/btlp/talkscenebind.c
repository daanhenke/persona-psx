/* Persona 1 (JP) - the negotiation scene that can leave the demons bound.
 *   0x80072324 BtlTalkSceneBind    BTLP only.
 *
 * BtlTalkSceneStep runs this while the top of g_btl_talk_scene is 5. It is one
 * of four scenes of the same shape - the others can end in HAPPY, PANIC and
 * CHARM - and the stage below the top drives it a step at a time.
 *
 * The running stage seeds the generator from the beam position and rolls for
 * what the demons do. Three even thirds: they run off, they hand something
 * over, or the contact is tried. Where the party is high enough for the Persona
 * on offer and has room for it, the last two thresholds are rewritten so a
 * single roll out of 256 hands the Persona over instead.
 *
 * The contact weighs the acting member's number against the demon's: at or
 * above it the negotiation ends with every demon the offer involved bound,
 * below it they are only left cowering. Either way the offer is marked so it
 * cannot be tried this way again.
 *
 * Handing an item over is the one outcome that can fail - with no room for it
 * scene 9 is pushed to say so - and every other outcome pops the level and
 * takes the negotiation down.
 */
#include <decomp/types.h>
#include <rand.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/offer.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/text.h>

/* g_btl_talk_stage: the running stage, then one per outcome of its roll. */
#define TALK_STAGE_FREE  0
#define TALK_STAGE_RUN   1
#define TALK_STAGE_GO    2
#define TALK_STAGE_ITEM  3
#define TALK_STAGE_TOUCH 4
#define TALK_STAGE_TAKE  5
#define TALK_STAGE_BACK  6

/* g_btl_talk_scene, for a level nothing is running on, and the one pushed when
   the item will not fit. */
#define TALK_SCENE_NONE 0xFF
#define TALK_SCENE_MORE 9

/* Which lines it says. The second argument is the scene's kind. */
#define TALK_MSG_KIND  5
#define TALK_MSG_TAKE1 4
#define TALK_MSG_TAKE2 5
#define TALK_MSG_ITEM  6

/* Where the boxes go. */
#define TALK_TEXT_X 0x28
#define TALK_TEXT_Y 0x92
#define TALK_BOX_W  0x11
#define TALK_BOX_X  0xA0

/* The pack BtlTalkEndStatus plays as the demons take the status. */
#define TALK_PACK_BIND 0xC

/* Which of the two offer levels the Persona is gated on here. */
#define TALK_OFFER_LEVEL 0

/* What BtlStockHasRoom answers when there is somewhere to put it. */
#define STOCK_HAS_ROOM 1

/* The outcomes the roll can reach, and how the level test opens the last. */
#define TALK_ODDS 4
#define TALK_ODDS_ALL    0x100
#define TALK_ODDS_ALMOST 0xFF

/* Set on the offer once it has been contacted this way. */
#define OFFER_CONTACTED 0x10000000

/* Bit 0 of g_btl_talk_flags: a contact has ended. */
#define TALK_FLAG_ENDED 1

/* Where the battle goes afterwards. */
#define BTL_PHASE_COMMAND 0
#define BTL_PHASE_WON     2
#define BTL_PHASE_OVER    3

extern const u_char *g_btl_talk_fled_script;
extern const u_char *g_btl_talk_bind_script;
extern const u_char *g_btl_talk_cowering_script;

extern int   VSync(int mode);
/* Its slot is a u_short, which is what reads g_btl_offer_slot unsigned here
   and signed where the offer itself is indexed. */
extern int   BtlOfferLevelTest(int level, u_short slot);
extern int   BtlTalkGiveItem(void);
extern const u_char *BtlMessage(int line, int kind);
extern void  BtlTalkEndStatus(u_char status, int pack);
extern void  BtlTalkEndStep(void);
extern void  BtlTakeOffer(u_short persona, const u_char *script);
extern void  BtlBoxOpen(short cols, short x, short y, int style);
extern void  BtlShowAilmentMarks(int show);

void BtlTalkSceneBind(void)
{
    /* Kept in .rodata and copied onto the stack on every call, which is what
       lets the level test below rewrite two of them. */
    int  odds[TALK_ODDS] = { 0x56, 0xAB, TALK_ODDS_ALL, 0 };
    int  roll;
    int  i;
    int *p;

    switch (g_btl_talk_stage[g_btl_talk_depth - 1]) {
    case TALK_STAGE_RUN:
        srand(VSync(-1));
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
        g_btl_talk_stage[g_btl_talk_depth - 1] = i + TALK_STAGE_GO;
        break;

    case TALK_STAGE_GO:
        BtlTalkersLeave();
        BtlTextOpen(g_btl_talk_fled_script, TALK_TEXT_X, TALK_TEXT_Y);
        BtlBoxOpen(TALK_BOX_W, TALK_BOX_X, TALK_TEXT_Y, 0);
        BtlWaitAnyKey();
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
        g_btl_phase = BTL_PHASE_OVER;
        break;

    case TALK_STAGE_ITEM:
        g_btl_talk_stage[g_btl_talk_depth - 1] = TALK_STAGE_GO;
        BtlSeqRun();
        BtlSeqPlay(BtlMessage(TALK_MSG_ITEM, TALK_MSG_KIND));
        BtlSeqRun();
        if (BtlTalkGiveItem() == 0) {
            g_btl_talk_scene[g_btl_talk_depth] = TALK_SCENE_MORE;
            g_btl_talk_stage[g_btl_talk_depth] = TALK_STAGE_RUN;
            g_btl_talk_depth++;
        }
        break;

    case TALK_STAGE_TOUCH:
        g_btl_talk_stage[g_btl_talk_depth - 1] = TALK_STAGE_BACK;
        if (g_btl_actors[g_btl_actor_slot].unk3A
            >= g_btl_enemies[g_btl_talk_target].unk3A) {
            BtlEndTalking();
            BtlTalkEndStatus(BTL_STATUS_BIND, TALK_PACK_BIND);
            BtlTextOpen(g_btl_talk_bind_script, TALK_TEXT_X, TALK_TEXT_Y);
            BtlBoxOpen(TALK_BOX_W, TALK_BOX_X, TALK_TEXT_Y, 0);
            BtlWaitAnyKey();
            BtlTalkEndStep();
        } else {
            BtlTextOpen(g_btl_talk_cowering_script, TALK_TEXT_X, TALK_TEXT_Y);
            BtlBoxOpen(TALK_BOX_W, TALK_BOX_X, TALK_TEXT_Y, 0);
            BtlWaitAnyKey();
        }
        g_btl_talk_flags |= TALK_FLAG_ENDED;
        g_btl_offer[g_btl_offer_slot].kinds |= OFFER_CONTACTED;
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

    case TALK_STAGE_BACK:
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
    }
}
