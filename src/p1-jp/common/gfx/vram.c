/* Persona 1 (JP) - blank a VRAM rectangle.
 *
 * Compiled into three overlays rather than called across the boundary:
 *   DNG @ 0x8008F4CC
 *   ADV @ 0x8008B2AC
 *   S2D @ 0x8007F9DC
 *
 * The display is masked off for the clear and the three VSyncs bracket it, so
 * this is the "wipe a texture page before loading into it" path rather than
 * anything per-frame.
 */
#include <types.h>
#include <libgpu.h>
#include <libetc.h>

/* Clears the rectangle to black. The do/while (0) wrapper - most likely a
   multi-statement macro in the original - has to stay; unwrapping it breaks
   the match. */
void VramClearRect(short x, short y, short w, short h)
{
    RECT r;

    do {
        r.x = x;
        r.y = y;
        r.w = w;
        r.h = h;
        VSync(0);
        SetDispMask(0);
        ClearImage(&r, 0, 0, 0);
        DrawSync(0);
        VSync(0);
        VSync(0);
    } while (0);
}
