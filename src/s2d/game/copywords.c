/* Persona 1 (JP) - forward copy of a count of 32-bit words.
 * Preserve forward-copy order, including overlapping source/destination.
 */
#include <decomp/types.h>

void func_80096DE4(u_long *dst, u_long *src, u_long count)
{
    while (count != 0) {
        *dst++ = *src++;
        count--;
    }
}
