/* Persona 1 (JP) - deferred VRAM uploads.
 *
 * Compiled into three overlays rather than called across the boundary:
 *   DNG @ 0x80077AFC / 0x80077B74
 *   ADV @ 0x8006839C / 0x80068414
 *   S2D @ 0x80067B24 / 0x80067B9C
 * Each overlay owns its own queue, so the addresses differ; the code does not.
 *
 * Callers hand over a rectangle and a pointer during the frame, and the whole
 * batch goes to VRAM in one place later. LoadImage has to be paired with a
 * DrawSync, so batching them keeps the stalls together instead of scattered
 * through the frame.
 */
#include <types.h>
#include <libgpu.h>

typedef struct {
    /* 0x00 */ u_long *data;
    /* 0x04 */ RECT    rect;
} ImageUpload;                  /* 0x0C bytes */

extern int         g_image_queue_count;
extern ImageUpload g_image_queue[];

/* The rectangle is copied a short at a time rather than by struct assignment:
   the original reads four u16s out of the caller's RECT and stores them
   individually, which is what a `short *` source produces. */
void QueueImageUpload(u_short *rect, u_long *data)
{
    int n;

    n = g_image_queue_count;
    g_image_queue[n].data = data;
    g_image_queue[n].rect.x = rect[0];
    g_image_queue[n].rect.y = rect[1];
    g_image_queue[n].rect.w = rect[2];
    g_image_queue_count = n + 1;
    g_image_queue[n].rect.h = rect[3];
}

/* Drains back to front. The count is written back before the upload, not
   after, so an entry queued from underneath one of these calls would still be
   picked up by the same pass.
 *
 * A plain `while` on the count, not a hoisted counter: gcc rotates it into a
 * guard test plus a peeled first `n = count - 1`, which is why the original
 * loads the count twice before the loop and once at the bottom. Caching it in
 * a local collapses those to one load and caps the match at 89.5%. */
void FlushImageUploads(void)
{
    int n;

    while (g_image_queue_count != 0) {
        n = g_image_queue_count - 1;
        g_image_queue_count = n;
        LoadImage(&g_image_queue[n].rect, g_image_queue[n].data);
        DrawSync(0);
    }
}
