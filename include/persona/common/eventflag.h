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

/* The flags themselves, a bit each, reached by literal address. */
#define g_event_flags ((u_char *)0x801F29C8)

/* Whether story flag `id` is set, tested in place the way the ADV loaders do
   it: the id is a signed short, so the byte index is a signed divide. */
#define EVENT_FLAG(id) \
    ((u_char)(g_event_flags[(short)(id) / 8] & (1 << ((id) & 7))))

#endif
