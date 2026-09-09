/* Persona 1 (JP) - freeing one timed image animation channel.
 *
 * Sixteen channels, each playing a list of {image, delay} pairs into one fixed
 * VRAM rectangle. ImageAnimStep walks the lists once a frame and hands each
 * due frame to QueueImageUpload, so an animation costs one queue entry per
 * frame rather than a redraw. A script pointer of -1 marks a free channel.
 *
 *   DNG 0x800778AC   ADV 0x80068154   S2D 0x800678D4
 *
 * Starting one is a unit of its own in imageanimstart.c. Between the two, ADV
 * carries a routine that frees every channel at once (imageanimall.c); the
 * other two overlays have something else there.
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

void ImageAnimStop(int chan)
{
    g_image_anim[chan].script = IMAGE_ANIM_FREE;
}
