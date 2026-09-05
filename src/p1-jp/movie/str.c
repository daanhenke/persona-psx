/* Persona 1 (JP) - MOVIE.EXE - streamed video decode @ 0x800814B4 */
#include <types.h>
#include <persona/common/str.h>

/* Decodes one streamed frame into the back buffer and releases the ring slot.
   Gives up after 0x800000 polls so a stalled stream cannot hang the loop. */
void StrDecodeNextFrame(StrDecodeTarget *target)
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
    target->index = (target->index == 0);
    DecDCTvlc(bs, target->buf[target->index]);
    StFreeRing(bs);

out:
    return;
}
