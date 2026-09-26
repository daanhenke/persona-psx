/* Persona 1 (JP) - freeing every timed image animation channel, DNG's way.
 *   DNG 0x800778D0
 *
 * ADV clears the channels' script pointers itself (src/common/gfx/
 * imageanimall.c); DNG's copy calls ImageAnimStop on each, from the top down.
 * It sits between the two halves of imageanim.c, as ADV's does.
 */
#include <decomp/types.h>
#include <persona/common/imageanim.h>

void ImageAnimStopAll(void)
{
    int chan;

    chan = IMAGE_ANIM_COUNT;
    do {
        chan--;
        ImageAnimStop(chan);
    } while (chan != 0);
}
