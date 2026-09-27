/* cc1flags: -O0 -G8 */
/* Persona 1 (JP) - OPEN.EXE, the opening movie @ 0x80088D3C
 *
 * Plays the opening movie off the disc once, or until START is pressed; the
 * player is Sony's streaming sample, as in ATLUS.EXE. Built without
 * optimisation like the rest of this executable.
 */
#include <decomp/types.h>
#include <libcd.h>
#include <libetc.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <libsnd.h>
#include <persona/common/str.h>
#include <persona/open/open.h>

extern int  StCdIntrFlag;
extern void StCdInterrupt(void);
extern void SsSetStereo(void);
extern void SsQuit(void);

void StrOutCallback(void);

/* Defined here: the unit reaches it gp-relative. */
int g_str_overrun;

/* Plays the movie and returns 1 if START cut it short, 0 if it ran out. */
int OpenPlayMovie(void)
{
    CdlFILE file;
    u_char  param;
    long    unused[2];
    RECT    rect;
    int     skipped;
    long    unused2[32];

    ResetCallback();
    while (CdInit() == 0) {
    }
    PadInit(0);
    ResetGraph(0);
    SsEnd();
    SsQuit();
    SsInit();
    SsSetStereo();
    SsSetSerialAttr(SS_SERIAL_A, SS_MIX, SS_SON);
    SsSetSerialVol(SS_SERIAL_A, 0x7FFF, 0x7FFF);
    setRECT(&rect, 0, 0, 1023, 511);
    ClearImage(&rect, 0, 0, 0);
    g_dec.vlc_id  = 0;
    g_dec.rect_id = 0;
    g_dec.done    = 0;
    while (CdSearchFile(&file, "\\STR1\\MV1C.STR;1") == 0) {
    }
    g_str_loc.minute = file.pos.minute;
    g_str_loc.second = file.pos.second;
    g_str_loc.sector = file.pos.sector;
    DecDCTReset(0);
    DecDCToutCallback(StrOutCallback);
    StSetRing(g_ring_buf, 20);
    StSetStream(1, 1, -1, 0, 0);
    StrSetDecEnv(&g_dec, 0, 0, 0, 240);
    GsInitGraph(320, 240, 4, 0, 1);
    GsDefDispBuff(g_dec.rect[0].x, g_dec.rect[0].y, g_dec.rect[1].x, g_dec.rect[1].y);
    SetDispMask(1);
    while (1) {
        g_movie_frame = 0;
        g_str_overrun = 0;
        StrKickCD(&g_str_loc);
        StrDecodeNextFrame(&g_dec);
        while (1) {
            DecDCTin(g_dec.vlc_buf[g_dec.vlc_id], 1);
            DecDCTout(g_dec.img_buf, g_dec.slice.w * g_dec.slice.h / 2);
            StrDecodeNextFrame(&g_dec);
            g_movie_frame++;
            StrSync(&g_dec);
            VSync(0);
            ResetGraph(1);
            GsSwapDispBuff();
            if (g_movie_frame >= 0x825) {
                skipped = 0;
                goto done;
            }
            if (g_str_overrun == 1) {
                skipped = 0;
                goto done;
            }
            if (PadRead(1) & PADstart) {
                skipped = 1;
                goto done;
            }
        }
    }
done:
    SsSetSerialVol(SS_SERIAL_A, 0, 0);
    param = CdlModeSpeed;
    while (CdControlB(CdlSetmode, &param, 0) == 0) {
    }
    DecDCToutCallback(0);
    CdDataCallback(0);
    CdReadyCallback(0);
    while (CdControlB(CdlPause, 0, 0) == 0) {
    }
    return skipped;
}

/* The MDEC's end-of-slice callback: loads the slice just decoded, then
   either starts the next one or, at the end of the area, flips the areas
   and flags the frame done. */
void StrOutCallback(void)
{
    if (StCdIntrFlag) {
        StCdInterrupt();
        StCdIntrFlag = 0;
    }
    LoadImage(&g_dec.slice, g_dec.img_buf);
    g_dec.slice.x += g_dec.slice.w;
    if (g_dec.slice.x < g_dec.rect[g_dec.rect_id].x + g_dec.rect[g_dec.rect_id].w) {
        DrawSync(0);
        DecDCTout(g_dec.img_buf, g_dec.slice.w * g_dec.slice.h / 2);
    } else {
        g_dec.done    = 1;
        g_dec.rect_id = (g_dec.rect_id == 0);
        g_dec.slice.x = g_dec.rect[g_dec.rect_id].x;
        g_dec.slice.y = g_dec.rect[g_dec.rect_id].y;
    }
}
