/* Persona 1 (JP) - setting a bank-3 flag.  ADV only.
 *   ADV 0x800B0A60
 *
 * Bank 3 takes a u_char id, so the index is an unsigned shift rather than the
 * signed division the other banks use. A unit of its own, well past the rest
 * of the source it shares.
 */
#include <decomp/types.h>

#define g_flags_bank3 ((u_char *)0x801F2A68)

void FlagBank3Set(u_char id)
{
    u_char *p;
    int     v;

    p = g_flags_bank3;
    v = p[id >> 3];
    v = v | (1 << (id & 7));
    p[id >> 3] = v;
}
