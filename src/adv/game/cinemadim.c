/* Persona 1 (JP) - dimming the room behind the cinema frame.  ADV only.
 *   0x800AF63C CinemaDim
 *
 * While the frame is up (g_actor_dim set) the room behind it is darkened a
 * step at a time: the backdrop layer, then every actor's sprite by a
 * sixteenth of its own brightness a step, and the frame's pieces to the
 * backdrop's level. A room whose backdrop is already black is left alone.
 */
#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <persona/common/bg.h>
#include <persona/adv/actor.h>

#define FRAME_SLOT 0x34
#define EXTRA_SLOT 0x40
#define EXTRA      25

extern void SlotSetBrightness(u_char slot, u_char level);

#define DIM(a, step) \
    (g_adv_actors[a].bright - (g_adv_actors[a].bright >> 4) * (step))

void CinemaDim(short step)
{
    int i;

    if (g_bg_layers[0].r != 0) {
        g_bg_layers[0].r = 0x80 - step * 8;
        g_bg_layers[0].g = 0x80 - step * 8;
        g_bg_layers[0].b = 0x80 - step * 8;
        for (i = 0; i < 8; i++) {
            SlotSetBrightness(i, DIM(i, step));
            SlotSetBrightness(i + 8, DIM(i + 8, step));
            SlotSetBrightness(i + 16, DIM(i + 16, step));
            SlotSetBrightness(i + EXTRA_SLOT, DIM(i + EXTRA, step));
            SlotSetBrightness(i + FRAME_SLOT, g_bg_layers[0].r);
        }
    }
}
