/* Persona 1 (JP) - play-time counter.
 *
 * Compiled into more than one overlay rather than called across the boundary:
 *   DNG @ 0x80075EA4
 *   ADV @ 0x80066420
 *   S2D @ 0x80065EB8
 * All three share the counter itself, which is in the work area.
 */
#include <types.h>

/* Four bytes: hours, minutes, seconds, frames. Everything but the hours rolls
   over at 60, and the clock stops at 99:59:59:59. Reached by hardcoded address
   rather than through a linker symbol. */
#define PLAYTIME_ADDR 0x801F29BC

void PlayTimeTick(void)
{
    u_char *t;

    t = (u_char *)PLAYTIME_ADDR;
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
