/* Persona 1 (JP) - drawing the full-screen effect that is running.  ADV only.
 *   0x800867BC AdvDrawEffect   0x8008686C AdvEffectNone
 *   0x80086874 EffectWaveBuild
 *
 * AdvRunFrame calls AdvDrawEffect once a frame; g_adv_effect picks which
 * effect draws. The colour fades are in screenfade.c and the sky in
 * effectsky.c.
 *
 * The wave shifts each of 64 strips of a picture sideways, the strips four
 * lines apart: a cosine running down the screen, its size wobbling with a
 * sine so no two frames bend the same way.
 */
#include <decomp/types.h>
#include <libgte.h>

typedef struct {
    short x;
    short y;
    short unk4;
    short unk6;
    void *cells;
} WaveRow;

#define WAVE_ROWS 64

extern u_char  g_adv_effect;
extern int     g_effect_tick;
extern WaveRow g_wave_rows[WAVE_ROWS];

extern void AdvEffectFadeRed(void);
extern void AdvEffectFadeGreen(void);
extern void AdvEffectFadeBlue(void);
extern void EffectSkyStep(void);
extern void EffectWaveSlow(void);
extern void EffectWaveFast(void);
extern void EffectColorCycle(int unused);

void AdvDrawEffect(void)
{
    switch (g_adv_effect) {
    case 1:
        AdvEffectFadeRed();
        break;
    case 2:
        AdvEffectFadeGreen();
        break;
    case 3:
        AdvEffectFadeBlue();
        break;
    case 4:
        EffectSkyStep();
        break;
    case 5:
        EffectWaveSlow();
        break;
    case 10:
        EffectWaveFast();
        break;
    case 13:
        EffectColorCycle(0);
        break;
    }
}

void AdvEffectNone(void)
{
}

void EffectWaveBuild(void)
{
    u_char i;
    int    x;

    for (i = 0; i < WAVE_ROWS; i++) {
        x = rcos(((g_effect_tick & 0x1FF) + i * 16) * 8);
        x = x / (rsin(((i + g_effect_tick) & 0xFF) * 16) / 100 + 200);
        g_wave_rows[i].x = x;
    }
}
