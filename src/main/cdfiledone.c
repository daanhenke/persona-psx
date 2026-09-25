/* Persona 1 (JP) - the completion callbacks the async CD reads install.
 *   SLPS_005.00 @ 0x800125E8 CdReadDoneCallback, 0x80012630 CdReadSeekCallback
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

/* The callback LoadFileToAddrAsyncSeek installs: on success it hands over to
   CdReadDoneCallback and seeks to g_cd_next_file, which that callback then
   reports the end of - unless the drive was released (-2) meanwhile. */
void CdReadSeekCallback(u_char status)
{
    if (status == CdlComplete) {
        CdReadCallback((CdlCB)0);
        if (g_cd_busy == -2) {
            g_cd_busy = -1;
        } else {
            CdSyncCallback(CdReadDoneCallback);
            while (!CdControlB(CdlSeekL, (u_char *)&g_cd_next_file, (u_char *)0))
                ;
        }
    } else if (status == CdlDiskError) {
        g_cd_busy = -3;
    }
}
