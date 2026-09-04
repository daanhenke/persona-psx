/* Persona 1 (JP) - END.EXE - streamed video decode @ 0x80082B24 */
#include <types.h>

/* Double-buffered MDEC target: `buf[index]` is the one currently being
   decoded into, and index flips on every frame. */
typedef struct {
    u_long *buf[2];
    int     index;
} StrDecodeTarget;

extern u_long *StrGetReadyFrame(void);
extern void    DecDCTvlc(u_long *bs, u_long *out);
extern void    StFreeRing(u_long *bs);

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
