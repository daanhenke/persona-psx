/* Persona 1 (JP) - the wavy battle transition.  DNG only.
 *   0x80071EF8 FieldWaveFxBegin
 *   0x80072018 FieldWaveFxRun
 *   0x80072064 FieldWaveFxStep
 */
#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <rand.h>
#include <persona/dng/field.h>

#define STRIP_COLUMNS 160

/* Sets the backdrop strip up and gives each column a stretch speed off a
   sine wave and a delay before it starts. The wave starts afresh with a
   random step, height and delay every time its phase passes 0x800, so the
   strip breaks into runs. */
void FieldWaveFxBegin(void)
{
    int i;
    int phase;
    int step, amp, delay;

    func_80065978();
    do {
        func_80065978();
    } while (g_draw_buf != 0);
    FieldInitStrip();
    phase = 0x801;
    for (i = 0; i < STRIP_COLUMNS; i++) {
        if (phase >= 0x801) {
            phase = 0;
            step = ((rand() & 0xF) + 5) * 11;
            amp = ((rand() & 0xF) + 1) * 2;
            delay = rand() & 0xF;
        }
        g_wave_speed[i] = (rsin(phase) * amp) >> 12;
        phase += step;
        g_wave_delay[i] = delay;
    }
    VSync(3);
    FieldClockTick(3);
    GsDefDispBuff(320, 256, 0, 240);
}

/* Sixty-five frames of the wave. */
void FieldWaveFxRun(void)
{
    int i;

    GsDefDispBuff(320, 256, 0, 240);
    for (i = 0; i < 65; i++) {
        FieldWaveFxStep();
    }
}

/* One frame of the wave: each column counts its delay down, then stretches
   downwards faster and faster, and every column fades. */
void FieldWaveFxStep(void)
{
    int i;

    g_draw_buf = GsGetActiveBuff();
    GsSetWorkBase(g_scene->packets[g_draw_buf]);
    GsClearOt(0, 0, &g_scene->ot[g_draw_buf]);
    for (i = 0; i < STRIP_COLUMNS; i++) {
        if (g_wave_delay[i] < 0) {
            g_scene->sprites[i].scaley += 10 + g_wave_speed[i];
        } else {
            g_wave_delay[i]--;
        }
        if (g_scene->sprites[i].r != 0) {
            g_scene->sprites[i].r -= 2;
        }
        if (g_scene->sprites[i].g != 0) {
            g_scene->sprites[i].g -= 2;
        }
        if (g_scene->sprites[i].b != 0) {
            g_scene->sprites[i].b -= 2;
        }
        GsSortSprite(&g_scene->sprites[i], &g_scene->ot[g_draw_buf], 0);
    }
    VSync(2);
    FieldClockTick(2);
    GsSwapDispBuff();
    GsSortClear(0, 0, 0, &g_scene->ot[g_draw_buf]);
    GsDrawOt(&g_scene->ot[g_draw_buf]);
}
