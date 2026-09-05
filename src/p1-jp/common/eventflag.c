/* Persona 1 (JP) - story event flags.
 *
 * Compiled into more than one binary rather than called across the boundary:
 *   SLPS_005.00 @ 0x800156AC
 *   S2D overlay @ 0x8009BC24
 *
 * One bit per flag in a byte array, inside the 0x8000-byte work area main
 * bzeros at boot. Read far more than written, and mostly from DNG and S2D:
 * these gate map and event content.
 */
#include <types.h>

/* A literal address, not an extern symbol - and that is not a shortcut. An
   indexed load through a linker symbol assembles to `addu $at,$at,rX`, through
   a literal to `addu $at,rX,$at`; the original is the second form, so the game
   reaches its work area by hardcoded address. Nothing here is named in the
   linked image. */
#define g_event_flags ((u_char *)0x801F29C8)

/* The id arrives by pointer, not by value - the callers keep it in a local and
   pass its address. */
int EventFlagTest(u_short *id)
{
    return (g_event_flags[*id >> 3] >> (*id & 7)) & 1;
}
