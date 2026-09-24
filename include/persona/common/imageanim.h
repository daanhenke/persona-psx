#ifndef PERSONA_COMMON_IMAGEANIM_H
#define PERSONA_COMMON_IMAGEANIM_H

/* Persona 1 (JP) - timed image animations.
 *
 * Sixteen channels, each playing a script into one fixed VRAM rectangle.
 * ImageAnimStep walks them once a frame and hands each due frame to
 * QueueImageUpload, so an animation costs one queue entry per frame rather
 * than a redraw.
 *
 * A script is a run of words: an image and the frames it stays up for, pair
 * after pair, ending in IMAGE_ANIM_END - or in IMAGE_ANIM_JUMP followed by
 * where to carry on, which is how a script loops.
 */
#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>

typedef struct {
    /* 0x00 */ u_long **script;   /* IMAGE_ANIM_FREE while the channel is idle */
    /* 0x04 */ u_long  *data;     /* image the current frame is uploading    */
    /* 0x08 */ int      delay;    /* frames left before the next entry       */
    /* 0x0C */ u_short  x;
    /* 0x0E */ u_short  y;
    /* 0x10 */ u_short  w;
    /* 0x12 */ u_short  h;
} ImageAnim;                      /* 0x14 bytes */

#define IMAGE_ANIM_COUNT 16
#define IMAGE_ANIM_FREE  ((u_long **)-1)

/* The two words a script can hold in place of an image. */
#define IMAGE_ANIM_END  ((u_long *)-1)
#define IMAGE_ANIM_JUMP ((u_long *)-3)

extern ImageAnim g_image_anim[IMAGE_ANIM_COUNT];

void ImageAnimStop(int chan);
void ImageAnimStopAll(void);
void ImageAnimStart(int chan, u_long **script, u_short x, u_short y,
                    u_short w, u_short h);
void ImageAnimStep(void);

/* The deferred VRAM uploads (imagequeue.c): the frame's LoadImages are
   batched here and flushed together, so their DrawSyncs come in one place. */
typedef struct {
    /* 0x00 */ u_long *data;
    /* 0x04 */ RECT    rect;
} ImageUpload;                  /* 0x0C bytes */

extern int         g_image_queue_count;
extern ImageUpload g_image_queue[];

/* Queues one upload of `data` into the rectangle x, y, w, h. */
void QueueImageUpload(u_short *rect, u_long *data);
void FlushImageUploads(void);

#endif
