/* Persona 1 (JP) - carrying out the talk act a member picked.  BTLP only.
 *   0x80069C44 BtlTalkSceneAct
 *
 * BtlTalkSceneStep runs this while the top of g_btl_talk_scene is 0x10. It
 * returns straight away while the choice box is still open, and does its work
 * once a choice is in - the box is dismissed and the frame loop held until the
 * menu has finished closing.
 *
 * The choice indexes a table in the scratch pack, 0x1E bytes per row of the box
 * and ten per choice. Each record holds two variants of the same act and the
 * offer decides which: sharing a bit with the record's mask takes the first,
 * not sharing it takes the second. The variant says which mood moves, by how
 * much, and which line is said.
 *
 * A move of ten or more is the demons noticing: it asks for a voice line and
 * lights the one being spoken to.
 *
 * What happens next is BtlOfferRank's answer. Anything but 1 just says the line
 * under scene 0xC. On 1 - this is the offer's best gauge - a big move into a
 * mood the offer actually wants ends the talking and hands over to
 * BtlTalkAnswer; anything else marks the gauge on the panel, remembers the act
 * and says the line as usual.
 */
#include <decomp/types.h>
#include <persona/btlp/offer.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/talk.h>

/* BtlMenuChoice's two non-answers. */
#define MENU_WAIT   (-0x100)
#define MENU_CANCEL (-1)

/* The acts sit past the choice rows BtlOpenChoice reads: the header word after
   g_btl_choice_table is their offset, and a row of the box is 0x1E of them. */
#define CHOICE_ROW 0x1E

/* A move this big is one the demons react to. */
#define ACT_NOTICED 10

/* Frames held either side of the box closing, and before the line. */
#define ACT_SETTLE 0xF
#define ACT_PAUSE  10

/* g_btl_talk_scene / g_btl_talk_stage. */
#define TALK_SCENE_NONE 0xFF
#define TALK_SCENE_WAIT 0xC
#define TALK_STAGE_FREE 0
#define TALK_STAGE_RUN  1

/* The panel that shows which gauges have filled. */
#define PANEL_GAUGES 1

/* What BtlOfferRank answers for the offer's best gauge. */
#define OFFER_RANK_BEST 1

/* Each ten-byte choice contains a mask, two moods, two amounts and two
   halfword line numbers. Keep byte-pointer reads: structure-member reads let
   gcc move the script lookup ahead of the scene-depth store. */
#define ACT_CHOICE_SIZE 10
#define ACT_MOOD         2
#define ACT_AMOUNT       4
#define ACT_LINE         6

/* Read the directory at each use so the lookup keeps the image's evaluation
   order. Each scene arm performs its own lookup and play; gcc joins the calls. */
#define ACT_SCRIPT(rec, which) \
    (BTL_SCRATCH + *(u_long *)BTL_SCRATCH \
     + *(u_long *)(BTL_SCRATCH + *(u_long *)BTL_SCRATCH \
                   + *(u_short *)((rec) + (which) * 2 + ACT_LINE) * 4))

extern int     g_btl_menu_aside;
extern u_char *g_btl_talk_said_script;

extern void BtlMenuAsideToggle(void);
extern int  BtlMenuChoice(void);
extern void BtlMenuDismiss(void);
extern int  BtlMenuState(void);
extern void BtlMenuUpdate(void);
extern void BtlTalkUpdate(void);
extern void BtlRefreshMoodGauges(void);
extern void BtlQueueVoice(u_char line, int alt);
extern void BtlHighlightBegin(int who);
extern void BtlTalkScoreLine(u_char mood, u_char amount);
extern int  BtlOfferRank(int slot);
extern void BtlPanelSetImage(int group, u_char image);
extern void BtlPushRecent(int value);
extern void BtlTalkAnswer(int slot, u_int act);

void BtlTalkSceneAct(void)
{
    u_char        *script;
    u_char        *rec;
    u_char        *row_start;
    int            choice;
    short         *mood_of;
    int            which;
    int            mood;
    u_int          bit;

    BtlMenuAsideToggle();
    choice = BtlMenuChoice();
    if (choice == MENU_WAIT) {
        return;
    }
    if (choice == MENU_CANCEL) {
        return;
    }
    BtlIndicatorClear();
    g_btl_menu_aside = 0;
    choice = BtlMenuChoice();
    mood_of = g_btl_offer[g_btl_offer_slot].mood;
    row_start = BTL_SCRATCH + g_btl_choice_row * CHOICE_ROW;
    rec = row_start + g_btl_choice_acts + choice * ACT_CHOICE_SIZE;
    BtlFaceClose();
    BtlRunFrames(ACT_SETTLE);
    BtlMenuDismiss();
    while (BtlMenuState() != 0) {
        BtlMenuUpdate();
        BtlTalkUpdate();
        BtlDrawFrame();
    }
    BtlRunFrames(ACT_SETTLE);

    which = (g_btl_offer[g_btl_offer_slot].flags & *(u_short *)rec) == 0;
    mood = *(rec + which + ACT_MOOD);
    mood_of += mood;
    *mood_of += *(rec + which + ACT_AMOUNT);
    BtlRefreshMoodGauges();
    if (*(rec + which + ACT_AMOUNT) >= ACT_NOTICED) {
        BtlQueueVoice(mood, 0);
    }
    if (*(rec + which + ACT_AMOUNT) != 0) {
        BtlHighlightBegin(mood);
    }
    g_btl_talk_said_script = ACT_SCRIPT(rec, which);
    BtlRunFrames(ACT_PAUSE);
    BtlTalkScoreLine(*(rec + which + ACT_MOOD),
                     *(rec + which + ACT_AMOUNT));

    if (BtlOfferRank(g_btl_offer_slot) == OFFER_RANK_BEST) {
        if (((short)g_btl_panel_gauges >> mood & 1) != 0) {
            if ((1 << mood & g_btl_offer[g_btl_offer_slot].kinds) != 0
                && *(rec + which + ACT_AMOUNT) >= ACT_NOTICED) {
                BtlSeqPlay(ACT_SCRIPT(rec, which));
                BtlSeqRun();
                BtlEndTalking();
                g_btl_talk_depth--;
                g_btl_talk_scene[g_btl_talk_depth] = TALK_SCENE_NONE;
                g_btl_talk_stage[g_btl_talk_depth] = TALK_STAGE_FREE;
                BtlTalkAnswer(g_btl_offer_slot, mood);
                return;
            }
        }
        if (((short)g_btl_panel_gauges >> mood & 1) == 0) {
            u_int kinds = g_btl_offer[g_btl_offer_slot].kinds;
            bit = 1 << mood;
            if ((bit & kinds) != 0
                && *(rec + which + ACT_AMOUNT) >= ACT_NOTICED) {
                g_btl_panel_gauges |= bit;
                BtlPanelSetImage(PANEL_GAUGES, g_btl_panel_gauges);
                BtlPushRecent(mood);
            }
        }
        g_btl_talk_depth--;
        g_btl_talk_scene[g_btl_talk_depth] = TALK_SCENE_NONE;
        g_btl_talk_stage[g_btl_talk_depth] = TALK_STAGE_FREE;
        g_btl_talk_scene[g_btl_talk_depth] = TALK_SCENE_WAIT;
        g_btl_talk_stage[g_btl_talk_depth] = TALK_STAGE_RUN;
        g_btl_talk_depth++;
        script = ACT_SCRIPT(rec, which);
        BtlSeqPlay(script);
    } else {
        g_btl_talk_depth--;
        g_btl_talk_scene[g_btl_talk_depth] = TALK_SCENE_NONE;
        g_btl_talk_stage[g_btl_talk_depth] = TALK_STAGE_FREE;
        g_btl_talk_scene[g_btl_talk_depth] = TALK_SCENE_WAIT;
        g_btl_talk_stage[g_btl_talk_depth] = TALK_STAGE_RUN;
        g_btl_talk_depth++;
        script = ACT_SCRIPT(rec, which);
        BtlSeqPlay(script);
    }
}
