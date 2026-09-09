/* Persona 1 (JP) - the event-flag getter outside the flags translation unit.
 *   DNG 0x8008BF50   ADV 0x8007D9B4   S2D 0x8007C3B4
 *
 * Identical to EventFlagGet; the name differs only because two functions
 * cannot share one in a single program. A unit of its own - the bank-2
 * collector that shares its source is far behind it, in flags2.c.
 */
#include <decomp/types.h>

#define g_event_flags ((u_char *)0x801F29C8)

int EventFlagGet2(short id)
{
    u_char *p;
    int     v;

    p = g_event_flags;
    v = p[id / 8];
    v = v & (1 << (id & 7));
    return v;
}
