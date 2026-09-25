/* Persona 1 (JP) - streaming a VAB body off the CD, and CD-DA playback.
 *   SLPS_005.00
 *   0x8001219C VabStreamLoad        0x80012290 VabStreamLoadAsync
 *   0x80012354 CdDaSetFile          0x800123C8 CdDaPlay
 *   0x80012470 CdDaPause            0x800124A0 CdDaResume
 *   0x80012524 VabStreamReadyCallback
 *
 * A VAB body is read sector by sector with CdlReadN, each one handed to
 * SsVabTransBodyPartly from the ready callback as it arrives. The CD-DA side
 * plays a track found by file name; CdDaReadyCallback (a unit of its own,
 * cddaready.c) loops or stops it at the end.
 */
#include <decomp/types.h>
#include <libcd.h>
#include <libsnd.h>
#include <persona/main/cd.h>
#include <persona/main/cdda.h>

/* Prototyped the way the SDK documents it. That does not match gcc's builtin,
   so the call below stays a call rather than being expanded inline. */
extern void *memcpy(u_char *dst, u_char *src, int n);

/* One sector, as CdGetSector hands it over. */
extern u_long g_cd_stream_buf[0x200];

void VabStreamReadyCallback(u_char status, u_char *result);

/* Streams `name` into VAB `vab` and waits for the transfer to finish. */
void VabStreamLoad(const char *name, short vab)
{
    CdlFILE file;
    u_char  param[8];

    while (g_cd_busy != -1)
        ;
    g_cd_stream_vab = vab;
    g_cd_busy = 0;

    CdSearchFileLoc(&file, name);
    param[0] = CdlModeSpeed;
    g_cd_stream_sectors = (file.size + 0x7FF) >> 11;
    while (!CdControlB(CdlSetmode, param, (u_char *)0))
        ;
    CdReadyCallback((CdlCB)VabStreamReadyCallback);
    while (!CdControlB(CdlReadN, (u_char *)&file, (u_char *)0))
        ;

    while (g_cd_busy != -1)
        ;
    SsVabTransCompleted(SS_WAIT_COMPLETED);
}

/* The same without waiting: g_cd_busy says when it is done. */
void VabStreamLoadAsync(const char *name, short vab)
{
    CdlFILE file;
    u_char  param[8];

    while (g_cd_busy != -1)
        ;
    g_cd_stream_vab = vab;
    g_cd_busy = 0;

    CdSearchFileLoc(&file, name);
    param[0] = CdlModeSpeed;
    g_cd_stream_sectors = (file.size + 0x7FF) >> 11;
    while (!CdControlB(CdlSetmode, param, (u_char *)0))
        ;
    CdReadyCallback((CdlCB)VabStreamReadyCallback);
    while (!CdControlB(CdlReadN, (u_char *)&file, (u_char *)0))
        ;
}

/* Where a CD-DA track starts and ends. Audio sectors are 2352 bytes. */
void CdDaSetFile(const char *name)
{
    CdlFILE file;

    CdSearchFileLoc(&file, name);
    memcpy((u_char *)&g_cd_da_start, (u_char *)&file.pos, sizeof(CdlLOC));
    CdIntToPos(CdPosToInt(&g_cd_da_start) + (file.size + 2351) / 2352, &g_cd_da_end_loc);
}

/* Plays the track `repeat` times; CDDA_REPEAT_FOREVER loops it. */
void CdDaPlay(int repeat)
{
    u_char param[8];

    g_cd_da_repeat = repeat;
    SsSetSerialAttr(SS_SERIAL_A, SS_MIX, SS_SON);
    SsSetSerialVol(SS_SERIAL_A, 0x7F, 0x7F);
    g_cd_da_end = CdPosToInt(&g_cd_da_end_loc);

    param[0] = CdlModeDA | CdlModeRept;
    while (!CdControlB(CdlSetmode, param, (u_char *)0))
        ;
    CdReadyCallback((CdlCB)CdDaReadyCallback);
    while (!CdControlB(CdlPlay, (u_char *)&g_cd_da_start, (u_char *)0))
        ;
}

void CdDaPause(void)
{
    while (!CdControlB(CdlPause, (u_char *)0, (u_char *)0))
        ;
}

/* Picks the track up where CdDaReadyCallback last saw the head. */
void CdDaResume(void)
{
    u_char param[8];

    SsSetSerialAttr(SS_SERIAL_A, SS_MIX, SS_SON);
    SsSetSerialVol(SS_SERIAL_A, 0x7F, 0x7F);

    param[0] = CdlModeDA | CdlModeRept;
    while (!CdControlB(CdlSetmode, param, (u_char *)0))
        ;
    CdReadyCallback((CdlCB)CdDaReadyCallback);
    while (!CdControlB(CdlPlay, (u_char *)&g_cd_da_pos, (u_char *)0))
        ;
}

/* Each sector as it arrives goes to the VAB; after the last one the drive is
   paused and released. */
void VabStreamReadyCallback(u_char status, u_char *result)
{
    if (g_cd_busy != 0) {
        return;
    }
    if (status != CdlDataReady) {
        return;
    }
    CdGetSector(g_cd_stream_buf, 0x200);
    SsVabTransBodyPartly((u_char *)g_cd_stream_buf, 0x800, g_cd_stream_vab);
    g_cd_stream_sectors -= 1;
    if (g_cd_stream_sectors <= 0) {
        CdReadyCallback((CdlCB)0);
        while (!CdControlB(CdlPause, (u_char *)0, (u_char *)0))
            ;
        g_cd_busy = -1;
    }
}
