/* Persona 1 (JP) - starting a scene's animated images.  ADV @ 0x80084598.
 *
 * Seven entries live in the scene pack, one per animation channel from 8
 * upwards. An entry with no script is skipped, and so is one the shown bit is
 * clear on; the rest are started where the entry says, at the size it says.
 *
 * An entry can also be made conditional, in which case the low nine bits of
 * its flags are an event flag and the image only appears once that flag is
 * set. That is how a scene gains a piece of scenery partway through the story
 * without the script having to place it.
 */
#include <types.h>

/* The entries sit in the scene pack, which is read to a fixed address. */
#define SCENE_IMAGES_AT 0x80100C40
#define SCENE_IMAGES    7

/* The channels these occupy. */
#define SCENE_IMAGE_CHAN 8

/* Bits of an entry's flags. */
#define IMG_SHOWN       0x4000
#define IMG_CONDITIONAL 0x8000
#define IMG_FLAG        0x1FF

/* An entry with no script left in it. */
#define IMG_NONE ((u_long **)-1)

typedef struct {
    /* 0x00 */ u_long **script;
    /* 0x04 */ short    x, y;
    /* 0x08 */ short    w, h;
    /* 0x0C */ u_short  flags;
    /* 0x0E */ u_char   pad0E[2];
} AdvSceneImage;                  /* 16 bytes */

#define g_scene_images ((AdvSceneImage *)SCENE_IMAGES_AT)

#define EVENT_FLAGS_AT 0x801F29C8

/* No prototype: the entry's shorts go out as plain ints. */
extern void ImageAnimStart();

void AdvSceneStartImages(void)
{
    AdvSceneImage *img;
    u_char        *event_flags;
    short          flags;
    int            one;
    int            n;
    u_char         i;

    i = 0;
    do {
        n   = i;
        img = &g_scene_images[n];
        if (img->script != IMG_NONE) {
            flags = g_scene_images[n].flags;
            if ((flags & IMG_SHOWN) != 0) {
                if ((flags & IMG_CONDITIONAL) != 0) {
                    /* The 1 is assigned where it is shifted, not before the
                       test - lifting it out costs a saved register. */
                    event_flags = (u_char *)EVENT_FLAGS_AT;
                    if ((event_flags[(flags & IMG_FLAG) / 8] &
                         ((one = 1) << (flags & 7))) == 0) {
                        goto next;
                    }
                }
                ImageAnimStart(n + SCENE_IMAGE_CHAN,
                               img->script,
                               g_scene_images[n].x, g_scene_images[n].y,
                               g_scene_images[n].w, g_scene_images[n].h);
            }
        }
    next:
        i++;
    } while (i < SCENE_IMAGES);
}
