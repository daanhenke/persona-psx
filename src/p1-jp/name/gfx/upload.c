/* Persona 1 (JP) - one VRAM upload, waited on.  NAME @ 0x800678EC.
 *
 * LoadImage has to be paired with a DrawSync; the overlays that upload during
 * a frame batch them through QueueImageUpload instead.
 */
#include <types.h>
#include <libgpu.h>

void UploadImage(int x, int y, int w, int h, u_long *data)
{
    RECT r;

    r.x = x;
    r.y = y;
    r.w = w;
    r.h = h;
    LoadImage(&r, data);
    DrawSync(0);
}
