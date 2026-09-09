/* Persona 1 (JP) - pointing a timed image animation channel at a script.
 *   DNG 0x80077908   ADV 0x800681A8   S2D 0x80067930
 *
 * A unit of its own: in DNG and S2D a routine that is not part of this file
 * sits between it and the channel-freeing half in imageanim.c.
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
