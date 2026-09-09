/* Persona 1 (JP) - one token of a message script.  BTLP only.
 *   0x8007BF28 BtlWindowType
 *
 * BtlWindowStep runs this while the window is in state 1. Anything but 0xFF is
 * a character and goes straight into the window; 0xFF introduces a control
 * code, and the byte after it says what to do. Codes that only change the
 * window's state return, and the ones that do not - a row break, a colour
 * change - fall through and the next token is read in the same frame.
 *
 * Seven of the codes point the window at one of BtlSetInsert's slots and put it
 * in state 7, which is what types a name or a number in the middle of a line.
 * They do not run in slot order; only these calls say which code is which slot.
 *
 * Four more ask for a voice line, numbered down from the code.
 *
 * The caller says whether the end of a line waits for a key or simply finishes,
 * which is the difference between a message the player reads and one the
 * sequencer is stepping through itself.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <persona/btlp/window.h>

/* What stands in for a character. */
#define TEXT_CODE 0xFF

/* The control codes. */
#define CODE_END      0x01   /* and 0xEF and 0xF5 - three spellings of it */
#define CODE_VOICE_LO 0xEB
#define CODE_VOICE_HI 0xEE
#define CODE_END2     0xEF
#define CODE_INSERT2  0xF0
#define CODE_CHOICE   0xF1
#define CODE_INSERT5  0xF2
#define CODE_INSERT3  0xF3
#define CODE_INSERT4  0xF4
#define CODE_END3     0xF5
#define CODE_ROW      0xF6
#define CODE_PAUSE    0xF7
#define CODE_INSERT0  0xF8
#define CODE_INSERT6  0xF9
#define CODE_INSERT1  0xFA
#define CODE_HOLD     0xFB
#define CODE_WAIT     0xFC
#define CODE_ANSWER   0xFD
#define CODE_ATTR     0xFE
#define CODE_SPEED    0xFF

/* Voice lines are numbered down from the top code. */
#define VOICE_TOP 0xEE

/* The states this puts the window in. */
#define WIN_DONE   0
#define WIN_DELAY  2
#define WIN_PAUSE  3
#define WIN_INSERT 7
#define WIN_KEY    8
#define WIN_ANSWER 10
#define WIN_CHOICE 0xB
#define WIN_HOLD   0xC
#define WIN_FULL   0xD

/* How long the pausing codes hold for. */
#define PAUSE_FRAMES 0xC
#define HOLD_FRAMES  10
#define WAIT_FRAMES  0x1E

/* The window holds fifteen characters to a row; past 0x2C it is full. */
#define WIN_ROW  15
#define WIN_LAST 0x2C

/* An insert slot, as BtlSetInsert fills it. */
typedef struct {
    /* 0x00 */ u_char cell[0x18];
} BtlInsert;                        /* 0x18 bytes */

extern BtlInsert g_btl_insert[];

extern const u_char *BtlWindowPutChar(BtlWindow *w, const u_char *p);
extern void          BtlIndicatorBar(void);
extern void          BtlQueueVoice(u_char line, int alt);

#ifdef NON_MATCHING
const u_char *BtlWindowType(BtlWindow *w, const u_char *p, int wait)
{
    const u_char *ins;
    u_char        c;
    int           placed;

    for (;;) {
        if (*p == TEXT_CODE) {
            /* p stays on the control code; each arm says how far past it the
               next token starts. */
            p++;
            c = *p;
            switch (c) {
            case CODE_END:
            case CODE_END2:
            case CODE_END3:
                if (wait != 0) {
                    w->state = WIN_KEY;
                    BtlIndicatorBar();
                    p++;
                    goto done;
                }
                w->state = WIN_DONE;
                p++;
                goto done;
            case CODE_WAIT:
                p++;
                w->state = WIN_DELAY;
                w->timer = WAIT_FRAMES;
                goto done;
            case CODE_SPEED:
                p++;
                w->state = WIN_DELAY;
                c = *p++;
                w->delay = c;
                w->timer = c;
                goto done;
            case CODE_HOLD:
                BtlIndicatorBar();
                p++;
                w->state = WIN_HOLD;
                w->timer = HOLD_FRAMES;
                goto done;
            case CODE_PAUSE:
                p++;
                w->state = WIN_PAUSE;
                w->timer = PAUSE_FRAMES;
                goto done;
            case CODE_ROW:
                placed = w->placed / WIN_ROW * WIN_ROW + WIN_ROW;
                w->placed = placed;
                p++;
                if (placed > WIN_LAST) {
                    BtlIndicatorBar();
                    w->state = WIN_FULL;
                    goto done;
                }
                continue;
            case CODE_ATTR:
                p++;
                w->attr = *p++;
                continue;
            case CODE_INSERT0:
                ins = g_btl_insert[0].cell;
                break;
            case CODE_INSERT3:
                ins = g_btl_insert[3].cell;
                break;
            case CODE_INSERT4:
                ins = g_btl_insert[4].cell;
                break;
            case CODE_INSERT5:
                ins = g_btl_insert[5].cell;
                break;
            case CODE_INSERT1:
                ins = g_btl_insert[1].cell;
                break;
            case CODE_INSERT6:
                ins = g_btl_insert[6].cell;
                break;
            case CODE_INSERT2:
                ins = g_btl_insert[2].cell;
                break;
            case CODE_ANSWER:
                p++;
                w->state = WIN_ANSWER;
                w->answer = *p++;
                BtlIndicatorBar();
                goto done;
            case CODE_CHOICE:
                w->state = WIN_CHOICE;
                BtlIndicatorBar();
                p += 2;
                goto done;
            case CODE_VOICE_LO:
            case CODE_VOICE_LO + 1:
            case CODE_VOICE_LO + 2:
            case CODE_VOICE_HI:
                BtlQueueVoice(VOICE_TOP - c, 0);
                p++;
                goto done;
            default:
                goto done;
            }
            w->text = ins;
            w->state = WIN_INSERT;
            p++;
        done:
            return p;
        }

        /* A plain character. The original puts this after the codes rather
           than before them. */
        p = BtlWindowPutChar(w, p);
        if (w->placed > WIN_LAST) {
            BtlIndicatorBar();
            w->state = WIN_FULL;
        }
        if (w->delay == 0) {
            return p;
        }
        w->state = WIN_DELAY;
        w->timer = w->delay;
        return p;
    }
}
#else
INCLUDE_ASM("btlp/nonmatchings/windowtype", BtlWindowType);
#endif

