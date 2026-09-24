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
#include <persona/common/imageanim.h>


void ImageAnimStop(int chan)
{
    g_image_anim[chan].script = IMAGE_ANIM_FREE;
}
