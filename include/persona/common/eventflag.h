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
#include <types.h>

/* The id arrives by pointer, not by value - the callers keep it in a local and
   pass its address. */
extern int EventFlagTest(u_short *id);

#endif
