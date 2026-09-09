/* Persona 1 (JP) - the completion callback the async CD reads install.
 *   SLPS_005.00 @ 0x800125E8
 *
 * A unit of its own; see cdfilesearch.c.
 */
#include <decomp/types.h>
#include <libcd.h>
#include <persona/main/cd.h>

/* Completion callback for the async path: clears itself and marks the drive
   idle on success, or records the error. Any other status is ignored. */
void CdReadDoneCallback(u_char status)
{
    if (status == CdlComplete) {
        CdReadCallback((CdlCB)0);
        g_cd_busy = -1;
    } else if (status == CdlDiskError) {
        g_cd_busy = -3;
    }
}
