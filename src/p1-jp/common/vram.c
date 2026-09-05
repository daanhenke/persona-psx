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

/* The do/while(0) is not decoration and not a permuter artefact that can be
   tidied away: it puts the body in its own basic block, which is what gets the
   RECT stores scheduled around the first VSync the way the original has them.
   A plain brace block does not do it - that scores 83.8%. The most likely
   reason the original looks like this is that the body was a multi-statement
   macro, which is exactly where this idiom comes from. */
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
