/* Persona 1 (JP) - freeing every timed image animation channel.  ADV only.
 *   ADV 0x80068178
 *
 * Sits between the two halves of imageanim.c in ADV; the other overlays have
 * an unrelated routine in the same place, which is why this is a unit of its
 * own.
 */
#include <decomp/types.h>
#include <persona/common/imageanim.h>


void ImageAnimStopAll(void)
{
    int chan;

    chan = IMAGE_ANIM_COUNT;
    do {
        chan--;
        g_image_anim[chan].script = IMAGE_ANIM_FREE;
    } while (chan != 0);
}
