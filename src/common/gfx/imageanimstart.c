/* Persona 1 (JP) - starting a timed image animation, and playing them all.
 *              DNG         ADV         S2D
 *   Start      0x80077908  0x800681A8  0x80067930
 *   Step       0x8007796C  0x8006820C  0x80067994
 *
 * A unit of its own: in DNG and S2D a routine that is not part of this file
 * sits between it and the channel-freeing half in imageanim.c.
 */
#include <decomp/types.h>
#include <persona/common/imageanim.h>


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

/* Once a frame: every busy channel whose delay has run out takes its next
   script entry - following a jump first, stopping at the end - queues that
   image into its rectangle and waits the entry's frames. The delay counts down
   on every busy channel, the one that just played included. */
void ImageAnimStep(void)
{
    u_long **s;
    u_long  *op;
    u_short  rect[4];
    int      i;

    for (i = 0; i < IMAGE_ANIM_COUNT; i++) {
        if (g_image_anim[i].script == IMAGE_ANIM_FREE) {
            continue;
        }
        if (g_image_anim[i].delay == 0) {
        next:
            s = g_image_anim[i].script;
            op = *s;
            if (op == IMAGE_ANIM_JUMP) {
                g_image_anim[i].script = s + 1;
                g_image_anim[i].script = (u_long **)s[1];
                goto next;
            }
            if (op == IMAGE_ANIM_END) {
                g_image_anim[i].script = IMAGE_ANIM_FREE;
            } else {
                g_image_anim[i].delay = (int)s[1];
                g_image_anim[i].data = s[0];
                rect[0] = g_image_anim[i].x;
                rect[1] = g_image_anim[i].y;
                rect[2] = g_image_anim[i].w;
                rect[3] = g_image_anim[i].h;
                QueueImageUpload(rect, g_image_anim[i].data);
                g_image_anim[i].script = s + 2;
            }
        }
        g_image_anim[i].delay--;
    }
}
