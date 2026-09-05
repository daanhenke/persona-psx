/* Persona 1 (JP) - timed image animation channels.
 *
 * Sixteen channels, each playing a list of {image, delay} pairs into one fixed
 * VRAM rectangle. ImageAnimStep walks the lists once a frame and hands each
 * due frame to QueueImageUpload, so an animation costs one queue entry per
 * frame rather than a redraw. A script pointer of -1 marks a free channel.
 *
 *                     DNG         ADV         S2D
 *   ImageAnimStop     0x800778AC  0x80068154  0x800678D4
 *   ImageAnimStopAll              0x80068178
 *   ImageAnimStart    0x80077908  0x800681A8  0x80067930
 */
#include <types.h>

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

void ImageAnimStop(int chan)
{
    g_image_anim[chan].script = IMAGE_ANIM_FREE;
}

void ImageAnimStopAll(void)
{
    int chan;

    chan = IMAGE_ANIM_COUNT;
    do {
        chan--;
        g_image_anim[chan].script = IMAGE_ANIM_FREE;
    } while (chan != 0);
}

/* Points a channel at a script and fixes the rectangle every frame of it lands
   in. The delay is cleared so the first entry plays on the next step. */
void ImageAnimStart(int chan, u_long **script, u_short x, u_short y,
                    u_short w, u_short h)
{
    g_image_anim[chan].script = script;
    g_image_anim[chan].x = x;
    g_image_anim[chan].y = y;
    g_image_anim[chan].delay = 0;
    g_image_anim[chan].w = w;
    g_image_anim[chan].h = h;
}
