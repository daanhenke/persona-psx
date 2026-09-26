/* cc1flags: -O0 -G8 */
/* Persona 1 (JP) - ATLUS.EXE - streamed video decode @ 0x8008114C
 *
 * Sony's streaming sample: strNextVlc, strNext, strSync, the default decode
 * environment and the CD kick. Built without optimisation and with a
 * small-data area: the original keeps a frame pointer and reaches the
 * overrun flag through $gp.
 */
#include <decomp/types.h>
#include <libcd.h>
#include <persona/common/str.h>

/* Defined here: the unit reaches it gp-relative. */
int g_str_overrun;

/* Decodes one streamed frame into the back buffer and releases the ring slot.
   Gives up after 0x800000 polls so a stalled stream cannot hang the loop. */
void StrDecodeNextFrame(StrDecodeTarget *dec)
{
    int     tries = 0x800000;
    u_long *bs;

    while ((bs = StrWaitFrame()) == NULL) {
        if (--tries == 0) {
            return;
        }
    }
    dec->vlc_id = dec->vlc_id ? 0 : 1;
    DecDCTvlc(bs, dec->vlc_buf[dec->vlc_id]);
    StFreeRing(bs);
    return;
}

/* Spins until the stream hands back a frame, then flags the end of the logo
   once the stream reaches its last frames. */
u_long *StrWaitFrame(void)
{
    u_long         *frame;
    StrFrameHeader *header;
    int             tries = 0x800000;

    while (StGetNext(&frame, (u_long **)&header)) {
        if (--tries == 0) {
            return NULL;
        }
    }
    if (header->frame_count >= 0xC4) {
        g_str_overrun = 1;
    }
    return frame;
}

/* Waits for the callback to finish the frame. If it never does, the frame is
   given up on as if it had: the display areas flip all the same. */
void StrSync(StrDecodeTarget *dec)
{
    int tries = 0x800000;

    while (dec->done == 0) {
        if (--tries == 0) {
            dec->done    = 1;
            dec->rect_id = dec->rect_id ? 0 : 1;
            dec->slice.x = dec->rect[dec->rect_id].x;
            dec->slice.y = dec->rect[dec->rect_id].y;
        }
    }
    dec->done = 0;
}

/* The two display areas at (x0, y0) and (x1, y1), 24-bit and 320 wide, with
   the slice at the first one. */
void StrSetDecEnv(StrDecodeTarget *dec, int x0, int y0, int x1, int y1)
{
    dec->vlc_buf[0] = g_vlc_buf0;
    dec->vlc_buf[1] = g_vlc_buf1;
    dec->vlc_id     = 0;
    dec->img_buf    = g_img_buf;
    dec->rect_id    = 0;
    dec->done       = 0;
    setRECT(&dec->rect[0], x0, y0, 480, 240);
    setRECT(&dec->rect[1], x1, y1, 480, 240);
    setRECT(&dec->slice, x0, y0, 24, 240);
}

/* Seeks to the movie and starts the stream read, retrying each until the
   drive takes it. */
void StrKickCD(CdlLOC *loc)
{
    while (!CdControl(CdlSeekL, (u_char *)loc, 0)) {
    }
    while (!CdRead2(CdlModeStream | CdlModeSpeed | CdlModeRT)) {
    }
}
