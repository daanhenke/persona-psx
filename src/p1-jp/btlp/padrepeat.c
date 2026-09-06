/* Persona 1 (JP) - what the battle reads instead of the raw pad.
 *   BTLP @ 0x8007A1A0
 *
 * Holding any of the four action buttons runs a counter up to thirty. The whole
 * held state is handed over on the frame the counter reads one - the initial
 * press - and again every three frames once it has stuck at thirty, which is
 * the auto-repeat and its delay. Every other frame delivers only newly pressed
 * buttons, with the four action buttons masked out so a press cannot arrive
 * twice.
 *
 * The four masks are variables because the pad configuration owns them, but
 * both configurations set these particular four the same way.
 */
#include <types.h>

#define BTL_HOLD_MAX     0x1E
#define BTL_REPEAT_DELAY 3

extern u_short g_btl_pad1;
extern u_short g_btl_pad1_edge;

extern u_short g_btl_key_square;
extern u_short g_btl_key_cross;
extern u_short g_btl_key_triangle;
extern u_short g_btl_key_circle;

extern int    g_btl_hold_frames;
extern int    g_btl_repeat_delay;
extern u_int  g_btl_input;

/* The four masks are read twice rather than kept in a local; hoisting them
   costs the match. */
#define BTL_ACTION_KEYS                                                       \
    (g_btl_key_triangle | g_btl_key_cross | g_btl_key_square | g_btl_key_circle)

void BtlPadRepeat(void)
{
    if ((g_btl_pad1 & BTL_ACTION_KEYS) == 0) {
        g_btl_hold_frames = 0;
        g_btl_repeat_delay = 0;
    } else {
        g_btl_hold_frames++;
        if (g_btl_hold_frames > BTL_HOLD_MAX) {
            g_btl_hold_frames = BTL_HOLD_MAX;
        }
    }

    if (g_btl_hold_frames == 1 || g_btl_hold_frames == BTL_HOLD_MAX) {
        int delay = g_btl_repeat_delay;
        g_btl_repeat_delay = delay - 1;
        if (delay == 0) {
            g_btl_input = g_btl_pad1;
            g_btl_repeat_delay = BTL_REPEAT_DELAY;
            return;
        }
    }
    g_btl_input = g_btl_pad1_edge & ~BTL_ACTION_KEYS;
}
