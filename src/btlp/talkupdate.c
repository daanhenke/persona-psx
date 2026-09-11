/* Persona 1 (JP) - one frame of the contact box.  BTLP only.
 *   0x8007B8A0 BtlTalkUpdate
 *
 * Four contact options in two columns of two, so up and down step by two and
 * wrap by stepping back, and left and right step by one within the row - the
 * parity of the index is which column it is on.
 *
 * The answer lands in g_btl_talk_choice, put back to BTL_TALK_WAIT every frame
 * so it is read exactly once. Cancel and the third key both back out; only
 * confirm answers with an option.
 *
 * The cursor follows the highlighted cell, one pixel higher than the choice
 * box puts it.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <persona/btlp/menu.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/input.h>
#include <persona/btlp/sound.h>

/* g_btl_talk_state: the box is only worked while it is taking the pad. */
#define BTL_TALK_LIVE 2

/* Nothing answered yet. */
#define BTL_TALK_WAIT (-0x100)

/* The grid. */
#define TALK_COLS    2
#define TALK_OPTIONS 4

/* Where the cursor sits on a cell. */
#define TALK_CURSOR_X0   0x10
#define TALK_CURSOR_STEP 8
#define TALK_CURSOR_Y0   0xAD

/* The clicks. */
#define TALK_SE_BANK    1
#define TALK_SE_MOVE    0
#define TALK_SE_CONFIRM 1
#define TALK_SE_CANCEL  2

extern BtlMenuCell g_btl_talk_cells[];
extern int         g_btl_talk_state;
extern int         g_btl_talk_index;
extern int         g_btl_talk_choice;


#ifdef NON_MATCHING
void BtlTalkUpdate(void)
{
    /* Three of the four moves work through a pointer of their own, which is
       what puts the index's address in a register for that block. */
    int *index;
    int *choice;
    int  was;

    switch (g_btl_talk_state) {
    case 0:
        break;

    case 1:
        break;

    case BTL_TALK_LIVE:
        if ((BtlInputKeys() & g_btl_key_up) != 0) {
            BtlSePlay(TALK_SE_BANK, TALK_SE_MOVE);
            was = g_btl_talk_index;
            g_btl_talk_index = was - TALK_COLS;
            if (was - TALK_COLS < 0) {
                g_btl_talk_index = was + TALK_COLS;
            }
        }
        if ((BtlInputKeys() & g_btl_key_down) != 0) {
            int previous;

            BtlSePlay(TALK_SE_BANK, TALK_SE_MOVE);
            do {
                index = &g_btl_talk_index;
            } while (0);
            previous = *index;
            *index = previous + TALK_COLS;
            if (*index >= TALK_OPTIONS) {
                *index = previous - TALK_COLS;
            }
        }
        if ((BtlInputKeys() & g_btl_key_left) != 0) {
            int *leftIndex;

            BtlSePlay(TALK_SE_BANK, TALK_SE_MOVE);
            leftIndex = &g_btl_talk_index;
            if ((*leftIndex & 1) != 0) {
                *leftIndex = *leftIndex - 1;
            } else {
                *leftIndex = *leftIndex + 1;
            }
        }
        if ((BtlInputKeys() & g_btl_key_right) != 0) {
            int previous;

            BtlSePlay(TALK_SE_BANK, TALK_SE_MOVE);
            do {
                index = &g_btl_talk_index;
            } while (0);
            previous = *index;
            *index = previous + 1;
            if ((*index & 1) == 0) {
                *index = previous - 1;
            }
        }

        choice = &g_btl_talk_choice;
        *choice = BTL_TALK_WAIT;
        if ((g_btl_pad1_edge & (g_btl_key_cancel | g_btl_key_abort)) != 0) {
            BtlSePlay(TALK_SE_BANK, TALK_SE_CANCEL);
            *choice = -1;
        } else if ((g_btl_pad1_edge & g_btl_key_confirm) != 0) {
            BtlSePlay(TALK_SE_BANK, TALK_SE_CONFIRM);
            *choice = g_btl_talk_index;
        }

        BtlCursorPlace(g_btl_talk_cells[g_btl_talk_index].x * TALK_CURSOR_STEP
                           + TALK_CURSOR_X0,
                       g_btl_talk_cells[g_btl_talk_index].y + TALK_CURSOR_Y0);
        break;
    }
}
#else
INCLUDE_ASM("btlp/nonmatchings/talkupdate", BtlTalkUpdate);
#endif
