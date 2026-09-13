/* Persona 1 (JP) - one frame of the two-option prompt.  BTLP only.
 *   0x800A3998 BtlChoiceUpdate
 *
 * `row` is one of the two sets' rows, and which one it is picks the set whose
 * frames are painted. Up or down flips the row, with a click. The first set
 * also keeps a help line up for the option that is up, unless the player has
 * turned help off. Both options are then repainted, the one that is up lit,
 * and the answer is read off the keys pressed this frame.
 */
#include <decomp/types.h>
#include <persona/btlp/choice.h>
#include <persona/btlp/input.h>
#include <persona/btlp/menu.h>
#include <persona/btlp/object.h>
#include <persona/btlp/pick.h>
#include <persona/btlp/sound.h>
#include <persona/btlp/text.h>

/* The answers a cancel and the third key give. */
#define CHOICE_CANCEL (-1)
#define CHOICE_ABORT  (-2)

int BtlChoiceUpdate(short *row)
{
    BtlObj **objs;
    int      level;
    int      i;

    objs = g_btl_choice1_objs;
    if (row == &g_btl_choice0_row) {
        objs = g_btl_choice0_objs;
    }
    if (BtlMenuKey() & (PAD_UP | PAD_DOWN)) {
        BtlSePlay(1, 0);
        *row ^= 1;
    }
    if (row == &g_btl_choice0_row) {
        if (g_btl_no_help == 0) {
            BtlOpenMessage(0, 0, g_btl_choice_help[g_btl_choice0_row],
                           PICK_HELP_X, PICK_HELP_Y);
        } else {
            BtlCloseMessage(0);
        }
    }
    for (i = 0; i < CHOICE_OPTIONS; i++) {
        level = CHOICE_DARK;
        if (i == *row) {
            level = CHOICE_LIT;
        }
        BtlObjSetRgb(objs[i]->attached, level, level, level);
        objs[i]->attached->fade = CHOICE_FADE;
    }
    if (g_btl_pad1_edge & g_btl_key_confirm) {
        return *row;
    }
    if (g_btl_pad1_edge & g_btl_key_cancel) {
        return CHOICE_CANCEL;
    }
    if (g_btl_pad1_edge & g_btl_key_abort) {
        return CHOICE_ABORT;
    }
    return BTL_PICK_WAIT;
}
