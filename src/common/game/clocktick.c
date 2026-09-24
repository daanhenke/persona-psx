/* Persona 1 (JP) - the event clock, counting up.
 *
 * Compiled into ADV and S2D rather than called across the boundary:
 *   ADV @ 0x800664E8
 *   S2D @ 0x80065F80
 * DNG runs the same four bytes as a countdown instead, in FieldClockTick.
 */
#include <decomp/types.h>

/* Four bytes: hours, minutes, seconds, frames, then the byte that says the
   clock is running. Everything but the hours rolls over at 60, and the clock
   stops at 99:59:59:59. Reached by hardcoded address rather than through a
   linker symbol. */
#define CLOCK_ADDR 0x801F29C0

void ClockTick(void)
{
    u_char *t;

    t = (u_char *)CLOCK_ADDR;
    if (t[4] == 0) {
        return;
    }
    if (t[3] == 59 && t[2] == 59 && t[1] == 59 && t[0] == 99) {
        return;
    }

    t[3]++;
    if (t[3] == 60) {
        t[3] = 0;
        t[2]++;
        if (t[2] == 60) {
            t[2] = 0;
            t[1]++;
            if (t[1] == 60) {
                t[1] = 0;
                t[0]++;
            }
        }
    }
}
