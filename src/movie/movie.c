/* Persona 1 (JP) - MOVIE.EXE, the FMV player.  @ 0x80080E3C
 *
 * main brings the hardware up, plays the movie the resident left in
 * g_movie_id and shuts everything down again before returning to it. The
 * player is Sony's streaming sample: the MDEC decodes one frame while the
 * callback loads the previous one's slices into the display area.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <libcd.h>
#include <libetc.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <libsnd.h>
#include <persona/common/eventflag.h>
#include <persona/common/str.h>
#include <persona/movie/movie.h>

extern int  StCdIntrFlag;
extern void StCdInterrupt(void);
extern void SsSetStereo(void);

extern void StrOutCallback(void);

/* Where the game resumes, reached by literal address. */
#define MAP_ID    (*(u_short *)0x801F5350)
#define MAP_POS_X (*(u_char *)0x801F5352)
#define MAP_POS_Y (*(u_char *)0x801F5353)
#define MAP_UNK4  (*(u_char *)0x801F5354)
#define MAP_ROOM  (*(u_char *)0x801F5355)

/* Defined here: the unit reaches them gp-relative. */
u_char g_movie_no;
u_char g_movie_first;
u_char g_movie_again;
int    g_movie_end;
int    g_movie_frame;
CdlLOC g_str_loc;

u_short g_movie_frames[] = {
    0x54,  0x54,  0x54,  0x54,  0x54,  0x54,  0x54,  0x5E,
    0x54,  0x54,  0x54,  0x68,  0xBE,  0xE2,  0xE2,  0xE2,
    0xE2,  0x3C1, 0x4CE, 0x2B2, 0x384, 0x31B, 0x330, 0x2FD,
    0xCF,  0x144, 0xC8,  0x4DD, 0x82A, 0x195, 0x37A, 0x1A4,
    0x195, 0x19A, 0x11D, 0x11D, 0x136, 0x21F, 0x1F9,
};

MovieAfter g_movie_after[] = {
    { 3, 0x40,  0x00, 0x00, 0, 0 }, /* 0x0C */
    { 0, 0x19,  0x12, 0x09, 0, 1 }, /* 0x0D */
    { 0, 0x1A,  0x08, 0x03, 0, 2 }, /* 0x0E */
    { 0, 0x1C,  0x01, 0x0B, 0, 0 }, /* 0x0F */
    { 0, 0x1D,  0x09, 0x12, 0, 3 }, /* 0x10 */
    { 5, 0x00,  0x00, 0x00, 0, 0 }, /* 0x11 */
    { 3, 0x03,  0x00, 0x00, 0, 0 }, /* 0x12 */
    { 3, 0x1D,  0x00, 0x00, 0, 0 }, /* 0x13 */
    { 3, 0x57,  0x00, 0x00, 0, 0 }, /* 0x14 */
    { 3, 0xB1,  0x00, 0x00, 0, 0 }, /* 0x15 */
    { 3, 0x103, 0x00, 0x00, 0, 0 }, /* 0x16 */
    { 3, 0x128, 0x00, 0x00, 0, 0 }, /* 0x17 */
    { 3, 0xC9,  0x00, 0x00, 0, 0 }, /* 0x18 */
    { 3, 0x99,  0x00, 0x00, 0, 0 }, /* 0x19 */
    { 2, 0x02,  0x1C, 0xAA, 0, 0 }, /* 0x1A */
    { 0, 0x1D,  0x09, 0x12, 0, 3 }, /* 0x1B */
    { 0, 0x1D,  0x09, 0x12, 0, 3 }, /* 0x1C */
    { 3, 0x86,  0x00, 0x00, 0, 0 }, /* 0x1D */
    { 0, 0x1D,  0x09, 0x12, 0, 3 }, /* 0x1E */
    { 0, 0x1D,  0x09, 0x12, 0, 3 }, /* 0x1F */
    { 3, 0x101, 0x00, 0x00, 0, 0 }, /* 0x20 */
    { 3, 0x1CD, 0x00, 0x00, 0, 0 }, /* 0x21 */
    { 3, 0xD5,  0x00, 0x00, 0, 0 }, /* 0x22 */
    { 3, 0x1CA, 0x00, 0x00, 0, 0 }, /* 0x23 */
    { 3, 0x90,  0x00, 0x00, 0, 0 }, /* 0x24 */
    { 3, 0xD9,  0x00, 0x00, 0, 0 }, /* 0x25 */
    { 3, 0x60,  0x00, 0x00, 0, 0 }, /* 0x26 */
};

int MoviePlay(void);

int main(void)
{
    u_char param;

    ResetGraph(0);
    PadInit(0);
    while (!CdInit()) {
    }
    ResetCallback();
    while (MoviePlay()) {
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

/* Plays g_movie_no to its end, then sets where the game resumes. */
#ifdef NON_MATCHING
int MoviePlay(void)
{
    CdlFILE          file;
    RECT             rect;
    char             name[] = "\\STR0\\MV00.STR;1";
    u_char           unused[0x74];

    SetDispMask(0);
    CdControlB(CdlPause, 0, 0);
    SsEnd();
    SsInit();
    if (g_options[0]) {
        SsSetStereo();
    } else {
        SsSetMono();
    }
    SsSetSerialAttr(SS_SERIAL_A, SS_MIX, SS_SON);
    SsSetSerialVol(SS_SERIAL_A, 0x7FFF, 0x7FFF);
    setRECT(&rect, 0, 0, 1023, 511);
    g_movie_first = g_movie_no = g_movie_id;
    ClearImage(&rect, 0, 0, 0);
    DecDCTReset(0);
    DecDCToutCallback(StrOutCallback);
    StSetRing(g_ring_buf, 20);
    StSetStream(1, 1, -1, 0, 0);
    StrSetDecEnv(&g_dec, 0, 0, 0, 240);
    GsInitGraph(320, 240, 4, 0, 1);
    GsDefDispBuff(g_dec.rect[0].x, g_dec.rect[0].y, g_dec.rect[1].x, g_dec.rect[1].y);
    SetDispMask(1);
    g_dec.vlc_id  = 0;
    g_dec.rect_id = 0;
    g_dec.done    = 0;

    for (;;) {
        StrPutHex(g_movie_no, &name[9], 2);
        StrPutHex(g_movie_no >> 4, &name[4], 1);
        while (!CdSearchFile(&file, name)) {
        }
        g_movie_frame = 0;
        g_movie_end   = 0;
        g_str_loc.minute = file.pos.minute;
        g_str_loc.second = file.pos.second;
        g_str_loc.sector = file.pos.sector;
        StrKickCD(&g_str_loc);
        StrDecodeNextFrame(&g_dec);
        do {
            DecDCTin(g_dec.vlc_buf[g_dec.vlc_id], 1);
            DecDCTout(g_dec.img_buf, g_dec.slice.w * g_dec.slice.h / 2);
            StrDecodeNextFrame(&g_dec);
            g_movie_frame++;
            StrSync(&g_dec);
            PadRead(1);
            VSync(0);
            ResetGraph(1);
            GsSwapDispBuff();
        } while (g_movie_frame < g_movie_frames[g_movie_no] - 3 && g_movie_end != 1);
        if (g_movie_again) {
            continue;
        }
        if (g_movie_again) {
            return 1;
        }
    if (g_movie_no >= 0xC) {
        g_movie_next_state = g_movie_after[g_movie_no - 0xC].state;
        MAP_ID   = g_movie_after[g_movie_no - 0xC].map;
        MAP_POS_X = g_movie_after[g_movie_no - 0xC].x;
        MAP_POS_Y = g_movie_after[g_movie_no - 0xC].y;
        MAP_UNK4  = g_movie_after[g_movie_no - 0xC].map_unk4;
        MAP_ROOM     = g_movie_after[g_movie_no - 0xC].room;
        if (g_movie_no == 0xC) {
            if (EVENT_FLAG(0x53)) {
                MAP_ID = 0xA0;
                MAP_ROOM   = 1;
            }
        } else if (g_movie_no == 0x25) {
            if (EVENT_FLAG(0x169)) {
                MAP_ID = 0xD8;
                MAP_ROOM   = 0;
            }
        } else {
            return 0;
        }
    } else {
        g_movie_next_state = 3;
        MAP_ID   = 0x10A;
        MAP_ROOM = 2;
    }
    return 0;
    }
}
#else
INCLUDE_ASM("movie/nonmatchings/movie", MoviePlay);
#endif

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
