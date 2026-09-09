/* Persona 1 (JP) - a u16 block copy.
 *
 * Compiled into three overlays rather than called across the boundary:
 *   DNG @ 0x80092AD4   ADV @ 0x8008EA34   S2D @ 0x80082FE8
 *
 * A unit of its own, sitting between the item routines in every overlay. It
 * came out of the same original source as the party slot helpers, which are in
 * party.c, partycompact.c and partyfind.c.
 */
#include <decomp/types.h>

/* Count is in entries, and compared signed - a zero or negative count copies
   nothing. */
void CopyShorts(u_short *src, u_short *dst, u_short count)
{
    int i;

    for (i = 0; i < count; i++) {
        *dst = *src;
        src++;
        dst++;
    }
}
