/* Persona 1 (JP) - the negotiation board's loops, and the stock list on it.
 * BTLP only.
 *   0x80076AFC BtlTalkBoardDone
 *   0x80076B0C BtlRunTalkBoard     0x80076BB8 BtlPlayTalkSeq
 *   0x80076C70 BtlRefreshStockList 0x80076D14 BtlBuildStockList
 *
 * BtlTalkBoardDone is the flag BtlRunTalkBoard waits on, for the callers
 * outside the loop.
 *
 * BtlRunTalkBoard puts the negotiation board up and turns frames over until
 * BtlTalkBoardStep says it is done: the board's effect is opened, selected and
 * given its kind, the cursor's flags are kept so the board can put them back,
 * and any key already pressed is thrown away first.
 *
 * BtlPlayTalkSeq pumps the demons' voices under the talk sequence until the
 * sequence stops, and then ends the talking. Two of the sequence's states are
 * answered as they come by: state 10 hands over the answer the sequence
 * settled on, and state 11 that the line was cut short; both put the
 * sequence back to waiting and bring the indicator bar up.
 *
 * The stock list is the twelve Persona stock slots as rows of an effect, two
 * rows a slot: the slot's stock byte drawn as a number, and the Persona's name
 * beside it. BtlBuildStockList chains all twenty-four onto the list's effect,
 * newest first, and BtlRefreshStockList colours them from what the stock now
 * holds - an empty slot is drawn grey and with the blank name.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/effect.h>
#include <persona/btlp/input.h>
#include <persona/btlp/talkboard.h>
#include <persona/common/persona.h>

#define STOCK_SLOTS 12

/* The number row and the name row: their kinds, lit and grey, and where they
   go. */
#define STOCK_NUMBER      0x42
#define STOCK_NUMBER_GREY 0x22
#define STOCK_NAME        0x01
#define STOCK_NAME_GREY   0x21
#define STOCK_NUMBER_X    3
#define STOCK_NAME_X      7
#define STOCK_BYTE        0xFF
#define STOCK_NUMBER_UNK  0x8E

/* The two sequence states the player answers on its way past. */
#define SEQ_ANSWER 10
#define SEQ_CUT    11
#define SEQ_WAIT   8

/* A number row with eight bytes of its own behind it. */
typedef struct {
    /* 0x00 */ BtlEffectRow row;
    /* 0x10 */ long         unk10;
    /* 0x14 */ long         unk14;
} BtlStockNumberRow;                   /* 0x18 bytes */

/* A text row, which has no use for the last word. */
typedef struct {
    /* 0x0 */ BtlEffectRow *next;
    /* 0x4 */ u_char        kind;
    /* 0x5 */ u_char        row;
    /* 0x6 */ u_char        x;
    /* 0x7 */ u_char        y;
    /* 0x8 */ const u_char *text;
} BtlStockNameRow;                     /* 0xC bytes */

extern BtlStockNumberRow g_btl_stock_numbers[STOCK_SLOTS];
extern BtlStockNameRow   g_btl_stock_list_names[STOCK_SLOTS];

extern void BtlUpdateVoices(void);
extern int  BtlSeqAnswer(void);
extern void BtlIndicatorBar(void);

int BtlTalkBoardDone(void)
{
    return g_btl_talk_board_done;
}

void BtlRunTalkBoard(void)
{
    int *slot;

    g_btl_talk_board_done = 0;
    g_btl_talk_board_step = 0;
    g_btl_talk_cursor_was = BtlCursorFlags();
    slot = &g_btl_talk_board_slot;
    BtlEffectSelect(*slot = BtlEffectOpen(&g_btl_talk_board));
    BtlEffectSetKind(*slot, 3);
    g_btl_pad1_edge = 0;
    while (g_btl_talk_board_done == 0) {
        BtlTalkBoardStep();
        BtlDrawFrame();
    }
}

void BtlPlayTalkSeq(void)
{
    while (1) {
        BtlUpdateVoices();
        if (BtlSeqState() == 0) {
            BtlEndTalking();
            return;
        }
        if (BtlSeqState() == SEQ_ANSWER) {
            g_btl_seq_answer = BtlSeqAnswer();
            BtlSeqSetState(SEQ_WAIT, 4);
            BtlIndicatorBar();
        }
        if (BtlSeqState() == SEQ_CUT) {
            g_btl_seq_cut = 1;
            BtlSeqSetState(SEQ_WAIT, 4);
            BtlIndicatorBar();
        }
        BtlDrawFrame();
    }
}

void BtlRefreshStockList(void)
{
    int i;

    i = 0;
    do {
        if (g_persona_stock[i] == 0) {
            g_btl_stock_numbers[i].row.kind = STOCK_NUMBER_GREY;
            g_btl_stock_list_names[i].kind = STOCK_NAME_GREY;
            g_btl_stock_list_names[i].text = g_btl_stock_blank;
        } else {
            g_btl_stock_numbers[i].row.kind = STOCK_NUMBER;
            g_btl_stock_list_names[i].kind = STOCK_NAME;
            g_btl_stock_list_names[i].text =
                g_persona_data[g_persona_stock[i]].name;
        }
        i++;
    } while (i < STOCK_SLOTS);
}

/* 95.66%: every store is where the image has it, but the image leaves the
   first constant the loop writes, STOCK_NUMBER_UNK, in the loop and lifts the
   next two, where gcc here lifts the first two and leaves the third. The head
   written through a local, the fields written through the row pointers, and a
   separate local for the next row number all move further away. */
#ifdef NON_MATCHING
void BtlBuildStockList(void)
{
    BtlStockNumberRow *num;
    BtlStockNameRow   *name;
    int                i;

    g_btl_stock_list.next = (BtlEffectRow *)-1;
    for (i = 0; i < STOCK_SLOTS; i++) {
        num = &g_btl_stock_numbers[i];
        name = &g_btl_stock_list_names[i];
        num->row.next = g_btl_stock_list.next;
        g_btl_stock_list.next = &num->row;
        name->next = g_btl_stock_list.next;
        g_btl_stock_list.next = (BtlEffectRow *)name;
        g_btl_stock_numbers[i].row.row = i;
        g_btl_stock_numbers[i].unk14 = STOCK_NUMBER_UNK;
        g_btl_stock_numbers[i].row.kind = STOCK_NUMBER;
        g_btl_stock_numbers[i].row.x = STOCK_NUMBER_X;
        g_btl_stock_numbers[i].row.y = i + 1;
        g_btl_stock_numbers[i].row.text = &g_persona_stock[i];
        g_btl_stock_numbers[i].row.u.mask = STOCK_BYTE;
        g_btl_stock_numbers[i].unk10 = 0;
        g_btl_stock_list_names[i].kind = STOCK_NAME;
        g_btl_stock_list_names[i].x = STOCK_NAME_X;
        g_btl_stock_list_names[i].row = STOCK_BYTE;
        g_btl_stock_list_names[i].y = i + 1;
    }
    BtlRefreshStockList();
}
#else
INCLUDE_ASM("btlp/nonmatchings/stocklist", BtlBuildStockList);
#endif
