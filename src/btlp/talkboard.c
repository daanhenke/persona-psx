/* Persona 1 (JP) - one frame of the negotiation board.  BTLP only.
 *   0x80076E50 BtlTalkBoardStep
 *
 * The board is a menu of pages, and this is the frame that drives it. At step
 * nought the board itself is up: R1 takes it down, and so does an answer of
 * -1, both by letting go of the board's effect, putting back whatever slot was
 * current and showing the cursor the way it was found. Any other answer opens
 * the page that entry names and moves the step to the entry plus one, so the
 * steps below line up with the entries above.
 *
 * Each page then reads its own key. The scene list and the message list are
 * answered by the cursor, the three editors by cross - and every one of them
 * lets go of the page, puts the slot back and returns the step to nought.
 *
 * The gauge editor works on a copy: the four gauges are widened into
 * g_btl_mood_shown on the way in and written back to the offer's own shorts
 * on the way out, where the panel is told about them again. The money editor
 * splits the purse into the thousands and what is under them, one number row
 * each, and multiplies them back together every frame.
 */
#include <decomp/types.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/effect.h>
#include <persona/btlp/input.h>
#include <persona/btlp/message.h>
#include <persona/btlp/offer.h>
#include <persona/btlp/panel.h>
#include <persona/btlp/talk.h>
#include <persona/btlp/talkboard.h>
#include <persona/common/item.h>

/* What each page is opened as. */
#define BOARD_PAGE_KIND 0x13
#define BOARD_LIST_KIND 1
#define BOARD_ITEM_KIND 3

/* The board's entries, which are also its steps less one. */
#define BOARD_SCENES 0
#define BOARD_MOODS  1
#define BOARD_LINES  2
#define BOARD_HELP   4
#define BOARD_MONEY  5
#define BOARD_STOCK  6
#define BOARD_ENTRIES 7

/* The steps. Four, five and eight are entries that take no page, and the
   switch still holds room for them. */
#define BOARD_STEPS 9

/* Two answers every page can give: nothing yet, and taken down. */
#define BOARD_NONE (-1)

/* What the message page announces itself with, and the item whose name it
   drops into the line. */
#define BOARD_INSERT_NUMBER 0
#define BOARD_INSERT_NAME   3
#define BOARD_NUMBER        0x309
#define BOARD_ITEM          1

/* Enough to stop the talk sequence wherever it had got to. */
#define BOARD_SEQ_STOP 0x3E7

/* What the money page splits the purse at. */
#define BOARD_MONEY_STEP 1000

/* Frames the scene page waits out once its pack is let go of. */
#define BOARD_CLOSE_FRAMES 3

extern int  BtlEffectOpen(BtlEffect *e);
extern int  BtlEffectAnswer(void);
extern void BtlEffectSetKind(int slot, u_char kind);

void BtlTalkBoardStep(void)
{
    int   *slot;
    short  answer;
    int    picked;
    /* One counter for both gauge walks. Two of them, one per block, would
       each be short-lived enough to outrank the pointer they walk beside,
       and the whole pre-header would come out in the other order. */
    int    i;

    switch (g_btl_talk_board_step) {
    case 0:
        if ((g_btl_pad1_edge & PAD_R1) != 0) {
            g_btl_talk_board_done = 1;
            g_btl_pad1_edge = 0;
            BtlEffectRelease(g_btl_talk_board_slot);
            BtlEffectRestore();
            BtlCursorShow(g_btl_talk_cursor_was);
            break;
        }
        answer = BtlEffectAnswer();
        if (answer == BTL_EFFECT_MARK) {
            break;
        }
        if (answer == BOARD_NONE) {
            g_btl_talk_board_done = 1;
            BtlEffectRelease(g_btl_talk_board_slot);
            BtlEffectRestore();
            BtlCursorShow(g_btl_talk_cursor_was);
            break;
        }
        switch (answer) {
        case BOARD_SCENES:
            g_btl_talk_board_step = answer + 1;
            {
                int *slot = &g_btl_talk_page_slot;

                BtlEffectSelect(*slot = BtlEffectOpen(&g_btl_talk_board_scenes));
                BtlEffectSetKind(*slot, BOARD_PAGE_KIND);
            }
            break;
        case BOARD_MOODS:
            g_btl_talk_board_step = answer + 1;
            {
                u_long *p;
                short **bar;

                i   = 0;
                p   = g_btl_mood_shown;
                bar = g_btl_mood_bar;
                do {
                    *p = **bar;
                    bar++;
                    i++;
                    p++;
                } while (i < BTL_MOODS);
            }
            {
                int *slot = &g_btl_talk_page_slot;

                BtlEffectSelect(*slot = BtlEffectOpen(&g_btl_talk_board_moods));
                BtlEffectSetKind(*slot, BOARD_LIST_KIND);
            }
            break;
        case BOARD_LINES:
            BtlSetInsert(BOARD_INSERT_NUMBER, (const u_char *)BOARD_NUMBER);
            BtlSetInsert(BOARD_INSERT_NAME, g_item_defs[BOARD_ITEM].name);
            g_btl_talk_board_step = answer + 1;
            {
                int *slot = &g_btl_talk_page_slot;

                BtlEffectSelect(*slot = BtlEffectOpen(&g_btl_talk_board_lines));
                BtlEffectSetKind(*slot, BOARD_ITEM_KIND);
            }
            break;
        case BOARD_HELP:
            slot = &g_btl_effect_held;
            if (*slot == BTL_EFFECT_FREE) {
                BtlEffectSetKind(*slot = BtlEffectOpen(&g_btl_talk_board_help),
                                 BOARD_PAGE_KIND);
            } else {
                g_btl_pad1 = 0;
                g_btl_pad1_edge = 0;
                BtlEffectRelease(*slot);
                *slot = BTL_EFFECT_FREE;
            }
            break;
        case BOARD_MONEY:
            g_btl_talk_board_step = answer + 1;
            {
                int *slot = &g_btl_talk_page_slot;

                BtlEffectSelect(*slot = BtlEffectOpen(&g_btl_talk_board_money));
                BtlEffectSetKind(*slot, BOARD_LIST_KIND);
            }
            g_btl_talk_board_money_low  = G_MONEY % BOARD_MONEY_STEP;
            g_btl_talk_board_money_high = G_MONEY / BOARD_MONEY_STEP;
            break;
        case BOARD_STOCK:
            g_btl_talk_board_step = answer + 1;
            BtlBuildStockList();
            {
                int *slot = &g_btl_talk_page_slot;

                BtlEffectSelect(*slot = BtlEffectOpen(&g_btl_stock_list));
                BtlEffectSetKind(*slot, BOARD_LIST_KIND);
            }
            break;
        }
        g_btl_pad1_edge = 0;
        break;

    case BOARD_SCENES + 1:
        answer = BtlEffectAnswer();
        if (answer == BTL_EFFECT_MARK) {
            break;
        }
        if (answer == BOARD_NONE) {
            BtlEffectRelease(g_btl_talk_page_slot);
            BtlEffectRestore();
            BtlDrawFrame();
            BtlDrawFrame();
            BtlDrawFrame();
            g_btl_pad1 = 0;
            g_btl_pad1_edge = 0;
            g_btl_talk_board_step = 0;
            break;
        }
        g_btl_pad1 = 0;
        g_btl_pad1_edge = 0;
        BtlEffectRelease(g_btl_talk_page_slot);
        BtlEffectRestore();
        g_btl_talk_board_step = 0;
        BtlLoadScratch(answer, 1);
        break;

    case BOARD_MOODS + 1:
        if ((g_btl_pad1_edge & PAD_CROSS) == 0) {
            break;
        }
        g_btl_pad1_edge = 0;
        BtlEffectRelease(g_btl_talk_page_slot);
        BtlEffectRestore();
        {
            u_long  *p;
            short  **bar;
            u_short  v;

            i   = 0;
            p   = g_btl_mood_shown;
            bar = g_btl_mood_bar;
            g_btl_talk_board_step = 0;
            do {
                v = *(u_short *)p;
                p++;
                **bar = v;
                bar++;
                i++;
            } while (i < BTL_MOODS);
        }
        BtlSetMoodGauges(*g_btl_mood_bar[0], *g_btl_mood_bar[1],
                         *g_btl_mood_bar[2], *g_btl_mood_bar[3]);
        break;

    case BOARD_LINES + 1:
        if ((g_btl_pad1_edge & PAD_CROSS) != 0) {
            g_btl_pad1_edge = 0;
            BtlEffectRelease(g_btl_talk_page_slot);
            BtlEffectRestore();
            g_btl_talk_board_step = 0;
        }
        picked = BtlEffectAnswer();
        if (picked == BOARD_NONE) {
            break;
        }
        if (picked < 0) {
            break;
        }
        if (picked != 0) {
            break;
        }
        g_btl_seq_answer = BOARD_SEQ_STOP;
        g_btl_seq_cut    = BOARD_SEQ_STOP;
        BtlTalkHide();
        {
            u_long  dir;
            u_long  at;
            u_char *script;

            dir = *(u_long *)BTL_SCRATCH;
            at  = *(u_long *)(BTL_SCRATCH + dir + g_btl_talk_board_line * 4);
            if (at == (u_long)BOARD_NONE) {
                BtlSeqPlay(g_btl_talk_board_blank_script);
            } else {
                script = at + BTL_SCRATCH;
                BtlSeqPlay(script + dir);
            }
        }
        BtlPlayTalkSeq();
        BtlCursorShow(1);
        break;

    case BOARD_MONEY + 1:
        G_MONEY = g_btl_talk_board_money_high * BOARD_MONEY_STEP
                  + g_btl_talk_board_money_low;
        if ((g_btl_pad1_edge & PAD_CROSS) == 0) {
            break;
        }
        g_btl_pad1_edge = 0;
        BtlEffectRelease(g_btl_talk_page_slot);
        BtlEffectRestore();
        g_btl_talk_board_step = 0;
        BtlInsertHeroName();
        break;

    case BOARD_STOCK + 1:
        BtlRefreshStockList();
        if ((g_btl_pad1_edge & PAD_CROSS) == 0) {
            break;
        }
        g_btl_pad1_edge = 0;
        BtlEffectRelease(g_btl_talk_page_slot);
        BtlEffectRestore();
        g_btl_talk_board_step = 0;
        break;

    case BOARD_STEPS - 1:
        break;
    }
}
