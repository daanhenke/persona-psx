#ifndef PERSONA_BTLP_TALKBOARD_H
#define PERSONA_BTLP_TALKBOARD_H

/* Persona 1 (JP) - the negotiation board.
 *
 * A board of its own, put up over the battle by BtlRunTalkBoard and turned
 * over a frame at a time until BtlTalkBoardStep says it is done. The board
 * itself is one effect, and each entry on it opens a second effect - a page -
 * beside it; g_btl_talk_board_step says which page is up and every page puts
 * the step back to nought as it closes.
 *
 * The step is the entry the board was answered with plus one, so the entries
 * and the steps line up: entry nought opens the scene list and step 1 reads
 * the chosen pack in, entry one opens the gauge editor and step 2 writes what
 * it left back into the offer, and so on. Entry four is the odd one out - it
 * holds a page open beside the board rather than taking a step of its own.
 */
#include <decomp/types.h>
#include <persona/btlp/effect.h>

/* The board, the slot it is open in, the slot whatever page is up is open in,
   the step, and the flag the loop outside waits on. */
extern BtlEffect g_btl_talk_board;
extern int       g_btl_talk_board_slot;
extern int       g_btl_talk_page_slot;
extern int       g_btl_talk_board_step;
extern int       g_btl_talk_board_done;

/* The cursor as it stood before the board went up. */
extern u_char g_btl_talk_cursor_was;

/* The pages, one per entry of the board. */
extern BtlEffect g_btl_talk_board_scenes; /* the scene packs, entry 0      */
extern BtlEffect g_btl_talk_board_moods;  /* the four gauges, entry 1      */
extern BtlEffect g_btl_talk_board_lines;  /* the messages, entry 2         */
extern BtlEffect g_btl_talk_board_help;   /* held open beside it, entry 4  */
extern BtlEffect g_btl_talk_board_money;  /* the purse, entry 5            */
extern BtlEffect g_btl_stock_list;        /* the Persona stock, entry 6    */

/* Which message the line page is on, and what the script it names is played
   as when the pack has none. */
extern int          g_btl_talk_board_line;
extern const u_char g_btl_talk_board_blank_script[];

/* The purse as the money page edits it: the thousands and what is under
   them, kept apart so each is a number row of its own. */
extern int g_btl_talk_board_money_low;
extern int g_btl_talk_board_money_high;

/* The gauges as the editor holds them, one word each, which BtlCopyMood fills
   from an offer's shorts. */
extern u_long g_btl_mood_shown[];

/* The blank a stock slot with nothing in it is drawn with. */
extern u_char g_btl_stock_blank[];

extern int  BtlTalkBoardDone(void);
extern void BtlRunTalkBoard(void);
extern void BtlPlayTalkSeq(void);
extern void BtlTalkBoardStep(void);
extern void BtlBuildStockList(void);
extern void BtlRefreshStockList(void);

#endif
