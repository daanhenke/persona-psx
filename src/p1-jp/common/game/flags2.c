/* Persona 1 (JP) - flag getters outside the flags translation unit.
 *
 * Both are static copies of the routine in flags.c, compiled into other units
 * and so present at their own addresses:
 *              DNG         ADV         S2D
 *   Bank 2     0x800991D0  0x80098A08  0x80089680
 *   Events     0x8008BF50  0x8007D9B4  0x8007C3B4
 *
 * The banks are in the shared save-game work area, so one source covers every
 * overlay. See flags.c for the layout of the three banks.
 */
#include <types.h>

#define g_event_flags ((u_char *)0x801F29C8)
#define g_flags_bank2 ((u_char *)0x801F2A48)

int FlagBank2Get(short id)
{
    u_char *p;
    int     v;

    p = g_flags_bank2;
    v = p[id / 8];
    v = v & (1 << (id & 7));
    return v;
}

/* Identical to EventFlagGet; the name differs only because two functions
   cannot share one in a single program. */
int EventFlagGet2(short id)
{
    u_char *p;
    int     v;

    p = g_event_flags;
    v = p[id / 8];
    v = v & (1 << (id & 7));
    return v;
}
