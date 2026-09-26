/* Persona 1 (JP) - MOVIE.EXE - streamed video decode @ 0x800814B4
 *
 * Sony's streaming sample, near enough line for line: strNext, strSync, the
 * default decode environment and the CD kick, plus the routine that writes a
 * movie number into the file name as hex digits.
 */
#include <decomp/types.h>
#include <libcd.h>
#include <persona/common/str.h>
#include <persona/movie/movie.h>

/* Defined here: the unit reaches them gp-relative. */
u_char g_movie_no;
int    g_movie_end;

/* Decodes one streamed frame into the back buffer and releases the ring slot.
   Gives up after 0x800000 polls so a stalled stream cannot hang the loop. */
void StrDecodeNextFrame(StrDecodeTarget *dec)
{
    u_long *bs;
    int     tries;

    tries = 0x800000;
poll:
    bs = StrGetReadyFrame();
    if (bs != NULL) {
        goto decode;
    }
    if (--tries == 0) {
        goto out;
    }
    goto poll;

decode:
    dec->vlc_id = (dec->vlc_id == 0);
    DecDCTvlc(bs, dec->vlc_buf[dec->vlc_id]);
    StFreeRing(bs);

out:
    return;
}

/* The next frame off the ring, or NULL once the stream has kept it waiting
   too long. A frame within three of the movie's last raises g_movie_end. */
u_long *StrGetReadyFrame(void)
{
    u_long         *addr;
    StrFrameHeader *header;
    int             tries;

    tries = 0x800000;
    while (StGetNext(&addr, (u_long **)&header)) {
        if (--tries == 0) {
            return NULL;
        }
    }
    if (header->frame_count >= g_movie_frames[g_movie_no] - 3) {
        g_movie_end = 1;
    }
    return addr;
}

/* Waits for the callback to finish the frame. If it never does, the frame is
   given up on as if it had: the display areas flip all the same. */
void StrSync(StrDecodeTarget *dec)
{
    int tries;

    tries = 0x800000;
    while (dec->done == 0) {
        if (--tries == 0) {
            dec->done = 1;
            dec->rect_id = (dec->rect_id == 0);
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
    while (CdControlB(CdlSeekL, (u_char *)loc, 0) == 0) {
    }
    while (CdRead2(CdlModeStream | CdlModeSpeed | CdlModeRT) == 0) {
    }
}

/* Writes the low `n` hex digits of `value` backwards, ending at `p`. */
void StrPutHex(short value, char *p, short n)
{
    short i;
    short d;

    for (i = 0; i < n; i++) {
        d = value % 16;
        value /= 16;
        *p = d < 10 ? d + '0' : d + '7';
        p--;
    }
}
