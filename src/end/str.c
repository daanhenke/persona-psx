/* Persona 1 (JP) - END.EXE - streamed video decode @ 0x80082B24 */
#include <decomp/types.h>
#include <persona/common/str.h>

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
