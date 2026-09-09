/* Persona 1 (JP) - freeing every timed image animation channel.  ADV only.
 *   ADV 0x80068178
 *
 * Sits between the two halves of imageanim.c in ADV; the other overlays have
 * an unrelated routine in the same place, which is why this is a unit of its
 * own.
 */
#include <decomp/types.h>

typedef struct {
    /* 0x00 */ u_long **script;   /* (u_long **)-1 while the channel is idle */
    /* 0x04 */ u_long  *data;     /* image the current frame is uploading    */
    /* 0x08 */ int      delay;    /* frames left before the next entry       */
    /* 0x0C */ u_short  x;
    /* 0x0E */ u_short  y;
    /* 0x10 */ u_short  w;
    /* 0x12 */ u_short  h;
} ImageAnim;                      /* 0x14 bytes */

#define IMAGE_ANIM_COUNT 16
#define IMAGE_ANIM_FREE  ((u_long **)-1)

extern ImageAnim g_image_anim[];

void ImageAnimStopAll(void)
{
    int chan;

    chan = IMAGE_ANIM_COUNT;
    do {
        chan--;
        g_image_anim[chan].script = IMAGE_ANIM_FREE;
    } while (chan != 0);
}
