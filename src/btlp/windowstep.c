/* Persona 1 (JP) - one frame of a message window.  BTLP only.
 *   0x8007C280 BtlWindowStep
 *
 * The window is a state machine and this runs it. Two states are handled
 * before the switch rather than inside it, because they are the ones a window
 * can be left in from the frame before: thirteen waits for the key that closes
 * a prompt, and fourteen slides the window off once it has been given.
 *
 * The rest go through the switch. Several only move the window and count a
 * timer down; the interesting ones are one, which walks the script, and seven,
 * which types the next character and stops when the script's terminator turns
 * up.
 *
 * It does not step once and return - it goes round until the window has
 * settled, which is what states nought and two mean. A caller that wants a
 * single frame's worth passes a non-zero `pause`, and the answer is whatever
 * state the window ended in.
 */
#include <decomp/types.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/input.h>
#include <persona/btlp/window.h>
#include <persona/btlp/sound.h>

/* The states. Eight, ten and eleven wait for the same key and go the same way
   afterwards; only the code that put the window in one of them tells them
   apart. */
#define WIN_ASK2    0xA
#define WIN_ASK3    0xB
#define WIN_CONFIRM 0xC
#define WIN_PROMPT  0xD   /* waiting on the key, before the switch */
#define WIN_CLOSING 0xE   /* sliding off after it, likewise       */

/* Any of these closes a prompt, not just the confirm key. */
#define WIN_ANY_KEY 0xF000

/* How far the window moves in a frame, and the two places it stops at. */
#define WIN_STEP    4
#define WIN_TOP     (-0x30)
#define WIN_TOP_END (-0x2F)

/* The slower scroll: two a frame, down to its own stop. */
#define WIN_SLIDE_STEP 2
#define WIN_SLIDE_END  (-0xF)
#define WIN_SLIDE_STOP (-0x10)

/* What each wait is given: thirty frames after a key, twelve to slide off, ten
   before the script picks up again. */
#define WIN_WAIT_KEY   0x1E
#define WIN_WAIT_CLOSE 0xC
#define WIN_WAIT_SEQ   0xA

/* The script's own terminator. */
#define WIN_SCRIPT_END 0xFF

/* The sound a key gets. */
#define WIN_SE_BANK  1
#define WIN_SE_SOUND 1

extern void          BtlSeqReset(void);
extern const u_char *BtlWindowPutChar(BtlWindow *w, const u_char *p);
extern const u_char *BtlWindowType(BtlWindow *w, const u_char *p, int wait);

short BtlWindowStep(BtlWindow *w, int pause)
{
    const u_char *s;

    s = w->script;
    /* A goto rather than a loop: written as one, gcc lifts the repeated
       state numbers into saved registers, which the original does not do. */
top:
    {
        if (w->state == WIN_PROMPT) {
            if ((g_btl_pad1_edge & (g_btl_key_confirm | WIN_ANY_KEY)) != 0) {
                BtlSePlay(WIN_SE_BANK, WIN_SE_SOUND);
                w->state = WIN_CLOSING;
                w->timer = WIN_WAIT_CLOSE;
            }
        }
        if (w->state == WIN_CLOSING) {
            w->y -= WIN_STEP;
            w->timer--;
            if (w->timer == 0) {
                BtlSeqReset();
                w->state = WIN_SCRIPT;
            }
        }

        /* Read unsigned: the range test the switch opens with is an unsigned
           one, which a plain short state does not give. */
        /* The arms are in the order the original's jump table lays their
           bodies out, which is not the order of the codes. */
        switch ((u_int)w->state) {
        case WIN_CONFIRM:
            if ((g_btl_pad1_edge & (g_btl_key_confirm | WIN_ANY_KEY)) != 0) {
                BtlSePlay(WIN_SE_BANK, WIN_SE_SOUND);
                w->state = WIN_SETTLE;
                w->timer = WIN_WAIT_KEY;
                BtlIndicatorClear();
            }
            break;
        case WIN_ASK:
        case WIN_ASK2:
        case WIN_ASK3:
            if ((g_btl_pad1_edge & (g_btl_key_confirm | WIN_ANY_KEY)) != 0) {
                BtlSePlay(WIN_SE_BANK, WIN_SE_SOUND);
                w->state = WIN_HOLD;
                w->timer = WIN_WAIT_KEY;
                BtlIndicatorClear();
            }
            break;
        case WIN_HOLD:
            w->timer--;
            if (w->timer == 0) {
                w->state = WIN_DONE;
            }
            break;
        case WIN_SETTLE:
            w->timer--;
            if (w->timer <= 0) {
                w->state = WIN_SCRIPT;
            }
            break;
        case WIN_CLOSE:
            w->y -= WIN_STEP;
            w->timer--;
            if (w->timer == 0) {
                BtlSeqReset();
                w->timer = WIN_WAIT_SEQ;
                w->state = WIN_SETTLE;
            }
            break;
        case WIN_RAISE:
            w->y -= WIN_STEP;
            if (w->y < WIN_TOP_END) {
                w->y = WIN_TOP;
                w->state = WIN_DONE;
                w->timer = 0;
            }
            break;
        case WIN_LOWER:
            w->y += WIN_STEP;
            if (w->y >= 0) {
                w->y = 0;
                w->state = WIN_DONE;
                w->timer = 0;
            }
            break;
        case WIN_SLIDE:
            w->slide -= WIN_SLIDE_STEP;
            if (w->slide < WIN_SLIDE_END) {
                w->slide = WIN_SLIDE_STOP;
                w->state = WIN_HOLD;
                w->timer = WIN_WAIT_KEY;
            }
            break;
        case WIN_TYPE:
            w->text = BtlWindowPutChar(w, w->text);
            if (*w->text == WIN_SCRIPT_END) {
                w->state = WIN_SCRIPT;
            }
            break;
        case WIN_SCRIPT:
            s = BtlWindowType(w, s, pause);
            break;
        case WIN_DONE:
            break;
        }
        w->script = s;
    }
    if (pause == 0 && w->state != WIN_DONE && w->state != WIN_SETTLE) {
        goto top;
    }
    return w->state;
}
