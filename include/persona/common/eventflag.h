#ifndef PERSONA_COMMON_EVENTFLAG_H
#define PERSONA_COMMON_EVENTFLAG_H

/* Persona 1 (JP) - story event flags.
 *
 * Compiled into more than one binary rather than called across the boundary
 * (SLPS_005.00 and the S2D overlay both carry a copy), and called from main's
 * preload code, so the prototype is shared. The flag array itself is reached by
 * literal address from inside src/p1-jp/common/eventflag.c and is not declared
 * here.
 */
#include <decomp/types.h>

/* The id arrives by pointer, not by value - the callers keep it in a local and
   pass its address. */
extern int EventFlagTest(u_short *id);

/* The second bank of them, which is the one the battle's winnings land in.
   Reached by literal address the same way the array itself is inside
   eventflag.c - the image loads it with lui/ori, not through the linker. */
#define EVENT_FLAGS_BANK2 ((u_long *)0x801F2A48)
#define EVENT_FLAG_BANK_WORDS 8

#endif
