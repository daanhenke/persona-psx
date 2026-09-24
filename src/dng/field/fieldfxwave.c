/* Persona 1 (JP) - the wavy battle transition.  DNG only.
 *   0x80071EF8 FieldWaveFxBegin
 *   0x80072018 FieldWaveFxRun
 */
#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <rand.h>
#include <persona/dng/field.h>

#define STRIP_COLUMNS 160

/* Sets the backdrop strip up and gives each column a sideways offset from
   a sine wave. The wave starts afresh with a random step, height and tint
   every time its phase passes 0x800, so the strip breaks into runs. */
void FieldWaveFxBegin(void)
{
    int i;
    int phase;
    int step, amp, tint;

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
            tint = rand() & 0xF;
        }
        g_wave_dx[i] = (rsin(phase) * amp) >> 12;
        phase += step;
        g_wave_tint[i] = tint;
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
        func_80072064();
    }
}
