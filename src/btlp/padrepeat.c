/* Persona 1 (JP) - what the battle reads instead of the raw pad.
 *   BTLP @ 0x8007A1A0
 *
 * Holding any direction runs a counter up to thirty. The whole held state is
 * handed over on the frame the counter reads one - the initial press - and
 * again every three frames once it has stuck at thirty, which is the
 * auto-repeat and its delay. Every other frame delivers only newly pressed
 * buttons, with the four directions masked out so a press cannot arrive twice.
 *
 * The four masks are variables because they sit in the table the control
 * scheme owns, but they are the one part of it neither scheme changes.
 */
#include <decomp/types.h>
#include <persona/btlp/input.h>


void BtlPadRepeat(void)
{
    if ((g_btl_pad1 & BTL_DIRECTIONS) == 0) {
        g_btl_hold_frames = 0;
        g_btl_repeat_delay = 0;
    } else {
        g_btl_hold_frames++;
        if (g_btl_hold_frames > BTL_HOLD_MAX) {
            g_btl_hold_frames = BTL_HOLD_MAX;
        }
    }

    if (g_btl_hold_frames == 1 || g_btl_hold_frames == BTL_HOLD_MAX) {
        int delay;

        delay = g_btl_repeat_delay;
        if (delay == 0) {
            g_btl_input = g_btl_pad1;
            g_btl_repeat_delay = BTL_REPEAT_DELAY;
            return;
        }
        g_btl_repeat_delay = delay - 1;
    }
    g_btl_input = g_btl_pad1_edge & ~BTL_DIRECTIONS;
}
