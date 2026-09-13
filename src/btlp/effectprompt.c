/* Persona 1 (JP) - the answer an effect gives, and two of its cursor
 * handlers.  BTLP only.
 *   0x800795D4 BtlEffectAnswer  0x80079600 BtlEffectCursorConfirm
 *   0x800796A0 BtlEffectCursorNumber
 *
 * An effect that asks something keeps its answer in the halfword at +4:
 * BTL_EFFECT_MARK while nothing has been decided, and whatever its cursor
 * handler writes once something has. BtlEffectAnswer is how the talk board
 * reads it.
 *
 * The two handlers are entries 1 and 2 of g_btl_effect_cursor_fn, and run for
 * the effect the pad is talking to. BtlEffectCursorConfirm clears the answer
 * every frame and then answers the cell the cursor is on for a confirm, or -1
 * for a cancel.
 *
 * BtlEffectCursorNumber edits the number the effect's current row points at.
 * A confirm starts the edit, which stops the grid cursor moving. While it
 * runs, the value is read at the width the row gives it, up and down step it
 * by one and right and left by ten, each held inside the row's bounds, and
 * either confirm or cancel ends the edit. Every frame of it the value is
 * written back at the same width.
 */
#include <decomp/types.h>
#include <persona/btlp/effect.h>
#include <persona/btlp/input.h>
#include <persona/btlp/sound.h>

/* The widths a number row's value can be. */
#define NUMBER_BYTE 0xFF
#define NUMBER_HALF 0xFFFF
#define NUMBER_WORD (-1)

/* How far right and left step the value. */
#define NUMBER_STRIDE 10

/* The answer a cancel gives. */
#define ANSWER_CANCEL (-1)

int BtlEffectAnswer(void)
{
    return g_btl_effect[g_btl_effect_cur]->answer;
}

void BtlEffectCursorConfirm(void)
{
    BtlEffect *e;

    e = g_btl_effect[g_btl_effect_cur];
    e->answer = BTL_EFFECT_MARK;
    if (BtlInputKeys() & g_btl_key_confirm) {
        BtlSePlay(1, 1);
        e->answer = e->sel;
    }
    if (BtlInputKeys() & g_btl_key_cancel) {
        BtlSePlay(1, 2);
        e->answer = ANSWER_CANCEL;
    }
}

void BtlEffectCursorNumber(void)
{
    BtlEffect          *e;
    BtlEffectNumberRow *step;
    long                value;

    e = g_btl_effect[g_btl_effect_cur];
    step = (BtlEffectNumberRow *)g_btl_effect_step[g_btl_effect_cur];
    if (e->kind & BTL_EFFECT_NOPAD) {
        switch (step->row.u.mask) {
        case NUMBER_BYTE:
            value = *(u_char *)step->row.text;
            break;
        case NUMBER_HALF:
            value = *(u_short *)step->row.text;
            break;
        case NUMBER_WORD:
            value = *(long *)step->row.text;
            break;
        }
        if (BtlInputKeys() & g_btl_key_up) {
            BtlSePlay(1, 0);
            if (++value > step->max) {
                value = step->max;
            }
        }
        if (BtlInputKeys() & g_btl_key_down) {
            BtlSePlay(1, 0);
            if (--value < step->min) {
                value = step->min;
            }
        }
        if (BtlInputKeys() & g_btl_key_right) {
            BtlSePlay(1, 0);
            if ((value += NUMBER_STRIDE) > step->max) {
                value = step->max;
            }
        }
        if (BtlInputKeys() & g_btl_key_left) {
            BtlSePlay(1, 0);
            if ((value -= NUMBER_STRIDE) < step->min) {
                value = step->min;
            }
        }
        if (BtlInputKeys() & g_btl_key_confirm) {
            BtlSePlay(1, 1);
            e->kind &= ~BTL_EFFECT_NOPAD;
        }
        if (BtlInputKeys() & g_btl_key_cancel) {
            BtlSePlay(1, 2);
            e->kind &= ~BTL_EFFECT_NOPAD;
        }
        switch (step->row.u.mask) {
        case NUMBER_BYTE:
            *(u_char *)step->row.text = 0;
            *(u_char *)step->row.text = value;
            break;
        case NUMBER_HALF:
            *(u_short *)step->row.text = 0;
            *(u_short *)step->row.text = value;
            break;
        case NUMBER_WORD:
            *(long *)step->row.text = 0;
            *(long *)step->row.text = value;
            break;
        }
    } else if (BtlInputKeys() & g_btl_key_confirm) {
        e->kind |= BTL_EFFECT_NOPAD;
    }
}
