/* cc1flags: -O0 -G8 */
/* Persona 1 (JP) - OPEN.EXE @ 0x80089430
 *
 * Built without optimisation and with a small-data area: the original keeps a
 * frame pointer and reaches the overrun flag through $gp, neither of which
 * -O2 -G0 would emit.
 */
#include <types.h>

/* Only the third word matters here: the decoded size of the frame. */
typedef struct {
    u_long unused[2];
    u_long size;
} StrFrameHeader;

extern int  StGetNext(u_long **frame, StrFrameHeader **header);

extern int g_str_overrun;

/* Spins until the stream hands back a frame, then flags any header whose
   third word has run past the expected size. */
u_long *StrWaitFrame(void)
{
    u_long          *frame;
    StrFrameHeader  *header;
    int              tries;

    tries = 0x800000;
    for (;;) {
        if (StGetNext(&frame, &header) == 0) {
            break;
        }
        if (--tries == 0) {
            return NULL;
        }
    }

    if (header->size >= 0x825) {
        g_str_overrun = 1;
    }
    return frame;
}
