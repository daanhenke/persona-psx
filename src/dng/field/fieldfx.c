/* Persona 1 (JP) - getting the field ready for a battle transition.
 * DNG only.
 *   0x800712B8 FieldFxBegin
 *   0x80071368 FieldFxRun
 */
#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <persona/dng/field.h>

/* Redraws until the first display buffer is the one being drawn, hands the
   scene's objects to the transition, and lets three frames pass on the
   clocks while it sets up. The display then covers 256 lines. */
void FieldFxBegin(void)
{
    func_80065978();
    do {
        func_80065978();
    } while (g_draw_buf != 0);
    g_fx_objs = g_scene->objs;
    g_fx_coords = g_scene->coords;
    g_fx_rots = g_scene->rots;
    func_8007192C();
    VSync(3);
    FieldClockTick(3);
    GsDefDispBuff(320, 256, 0, 240);
    g_fx_shift.vx = g_fx_shift.vy = g_fx_shift.vz = 0;
}

/* Sixty-four frames of transition `kind`. */
void FieldFxRun(int kind)
{
    int i;

    for (i = 0; i < 64; i++) {
        func_800713B0(kind);
    }
}
