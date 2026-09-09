/* Persona 1 (JP) - a single CD read, polled to completion.
 *   SLPS_005.00 @ 0x80012DC8
 *
 * A unit of its own; see cdfilesearch.c.
 */
#include <decomp/types.h>
#include <libcd.h>
#include <persona/main/cd.h>

/* Issues a single read and polls it to completion, pumping VSync while it
   waits. Returns the final CdReadSync status. */
int CdReadPolled(int size, u_long *dest, int mode)
{
    int res;

    CdRead((size + 0x7FF) / 2048, dest, mode);
    while ((res = CdReadSync(1, (u_char *)0)) > 0) {
        VSync(0);
    }
    return res;
}
