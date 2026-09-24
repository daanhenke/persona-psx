/* Persona 1 (JP) - shaking the view.  DNG only.
 *   0x8007398C FieldShake
 *   0x800739F8 FieldNudge
 */
#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <persona/dng/field.h>

/* Three quick jolts of the view, starting downwards when `dir` is -1 and
   upwards otherwise. */
void FieldShake(int dir)
{
    if (dir == -1) {
        FieldNudge(3, 3);
        FieldNudge(3, -3);
        FieldNudge(3, 3);
    } else {
        FieldNudge(3, -3);
        FieldNudge(3, 3);
        FieldNudge(3, -3);
    }
}

/* Moves the eye `dy` a frame for `frames` frames, redrawing each. */
void FieldNudge(int frames, int dy)
{
    int i;

    for (i = 0; i < frames; i++) {
        g_dng->view.vpy = dy + g_dng->view.vpy;
        func_80065978();
    }
}
