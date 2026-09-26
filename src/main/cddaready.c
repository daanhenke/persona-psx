/* Persona 1 (JP) - the CD-DA ready callback.  SLPS_005.00 @ 0x800129E4
 *
 * A unit of its own, after the queue's callbacks; CdDaPlay and CdDaResume in
 * cdstream.c install it. It keeps g_cd_da_pos current from each report, and
 * at the end of the track either plays it again or pauses.
 */
#include <decomp/types.h>
#include <libcd.h>
#include <persona/main/cdda.h>

void CdDaReadyCallback(u_char status, u_char *result)
{
    if (status != CdlDataReady) {
        return;
    }
    /* Bit 7 of the second's byte marks a report from outside the track. */
    if (result[4] & 0x80) {
        return;
    }

    g_cd_da_pos.minute = result[3];
    g_cd_da_pos.second = result[4];
    g_cd_da_pos.sector = result[5];
    if (CdPosToInt((CdlLOC *)&result[3]) < g_cd_da_end) {
        return;
    }

    g_cd_da_repeat &= ~CDDA_REPEAT_PASSED;
    if (g_cd_da_repeat == CDDA_REPEAT_FOREVER) {
        while (!CdControlB(CdlPlay, (u_char *)&g_cd_da_start, (u_char *)0))
            ;
    } else {
        g_cd_da_repeat -= 1;
        if (g_cd_da_repeat == 0) {
            while (!CdControlB(CdlPause, (u_char *)0, (u_char *)0))
                ;
        } else {
            while (!CdControlB(CdlPlay, (u_char *)&g_cd_da_start, (u_char *)0))
                ;
        }
    }
    g_cd_da_repeat |= CDDA_REPEAT_PASSED;
}
