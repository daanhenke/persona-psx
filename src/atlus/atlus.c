/* cc1flags: -O0 -G8 */
/* Persona 1 (JP) - ATLUS.EXE, the logo movie.  @ 0x80080BC8
 *
 * Plays \STR0\ATLUS.STR once, or until START is pressed, and returns to the
 * resident. The player is Sony's streaming sample, built without
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

extern int  StCdIntrFlag;
extern void StCdInterrupt(void);
extern void SsSetStereo(void);

void StrOutCallback(void);

/* Defined here: the unit reaches it gp-relative. */
int g_str_overrun;

int main(void)
{
    CdlFILE file;
    u_char  param;
    long    unused[2];
    RECT    rect;

    ResetGraph(0);
    PadInit(0);
    while (!CdInit()) {
    }
    ResetCallback();
    SetDispMask(0);
    SsInit();
    SsSetStereo();
    SsSetSerialAttr(SS_SERIAL_A, SS_MIX, SS_SON);
    SsSetSerialVol(SS_SERIAL_A, 0x7FFF, 0x7FFF);
    setRECT(&rect, 0, 0, 1023, 511);
    ClearImage(&rect, 0, 0, 0);
    g_dec.vlc_id  = 0;
    g_dec.rect_id = 0;
    g_dec.done    = 0;
    g_str_overrun = 0;
    while (!CdSearchFile(&file, "\\STR0\\ATLUS.STR;1")) {
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
    StrKickCD(&g_str_loc);
    StrDecodeNextFrame(&g_dec);
    while (1) {
        DecDCTin(g_dec.vlc_buf[g_dec.vlc_id], 1);
        DecDCTout(g_dec.img_buf, g_dec.slice.w * g_dec.slice.h / 2);
        StrDecodeNextFrame(&g_dec);
        StrSync(&g_dec);
        VSync(0);
        ResetGraph(1);
        GsSwapDispBuff();
        SetDispMask(1);
        if (g_str_overrun == 1) {
            break;
        }
        if (PadRead(1) & PADstart) {
            break;
        }
    }
    SetDispMask(0);
    SsSetSerialVol(SS_SERIAL_A, 0, 0);
    param = CdlModeSpeed;
    CdControlB(CdlSetmode, &param, 0);
    DecDCToutCallback(0);
    CdDataCallback(0);
    CdReadyCallback(0);
    CdControlB(CdlPause, 0, 0);
    ResetGraph(0);
    PadStop();
    StopCallback();
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
        g_dec.rect_id = g_dec.rect_id ? 0 : 1;
        g_dec.slice.x = g_dec.rect[g_dec.rect_id].x;
        g_dec.slice.y = g_dec.rect[g_dec.rect_id].y;
    }
}
