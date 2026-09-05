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

/* Appends one upload to the queue. The destination rectangle arrives as four
   u16s - x, y, w, h - and is copied field by field into the entry. */
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

/* Uploads the whole queue and empties it, most recently queued entry first,
   with a DrawSync after each one. The count is written back before the upload
   rather than after, so an entry queued from underneath one of these calls
   would still be picked up by the same pass. */
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
