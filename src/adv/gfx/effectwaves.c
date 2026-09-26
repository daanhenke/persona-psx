/* Persona 1 (JP) - the colour cycle and the two waves.  ADV only.
 *   0x80086DAC EffectColorCycle   0x80086F4C EffectWaveSlow
 *   0x80087094 EffectWaveFast
 *
 * Three of AdvDrawEffect's cases (13, 5 and 10), between the colour fades in
 * screenfade.c and the sky in effectsky.c.
 *
 * The colour cycle is 240 full-width lines, one a scanline, coloured out of
 * a ramp of 15-bit colours the scene pack carries, the start of the ramp
 * rolling up and down with a sine; every channel is scaled by the fade level.
 *
 * The waves bend the picture the effect slots show: it is cut into 61 strips
 * four lines tall, three banks of them side by side, and every frame each
 * strip is moved sideways by a cosine that runs down the screen and scrolled
 * along its texture. The two differ only in pace and in how far they bend.
 */
#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>

/* One strip: a 16-byte sprite record the slot draws from. */
typedef struct {
    u_char pad0[8];
    u_char u;
    u_char v;
    u_char pad0A[6];
} WaveStrip;

/* Where a strip is placed. */
typedef struct {
    short      x;
    short      y;
    short      unk4;
    short      unk6;
    WaveStrip *strip;
} WaveRow;

#define WAVE_STRIPS 61

#define g_cycle_lines ((GsLINE *)0x800ED180)
#define CYCLE_LINES   240
#define SCREEN_W      0x140
#define OT_BACK       0x400

extern int       g_effect_tick;
extern GsBG      g_bg_layers[];
extern GsOT      g_ot[];
extern int       g_ot_index;
extern u_short  *g_scene_ramp;
extern WaveStrip g_wave_strips[];
extern WaveRow   g_wave_bank_rows[];

void EffectColorCycle(int unused)
{
    GsLINE  *l;
    u_short *ramp;
    u_short *c;
    int      start;
    int      v;
    int      i;

    ramp = g_scene_ramp;
    l = g_cycle_lines;
    start = rsin((u_char)g_effect_tick * 16) / 512 + 8;
    for (i = 0; i < CYCLE_LINES; i++) {
        l->attribute = 0;
        l->x1 = SCREEN_W;
        /* An address sum rather than &ramp[...]: the offset comes first. */
        c = (u_short *)((i + start) * 2 + (int)ramp);
        l->x0 = 0;
        l->y0 = i;
        l->y1 = i;
        v = (*c & 0x1F) * g_bg_layers[0].r / 16;
        if (v > 0xFF) {
            v = 0xFF;
        }
        l->r = v;
        v = ((*c >> 5) & 0x1F) * g_bg_layers[0].r / 16;
        if (v > 0xFF) {
            v = 0xFF;
        }
        l->g = v;
        v = ((*c >> 10) & 0x1F) * g_bg_layers[0].r / 16;
        if (v > 0xFF) {
            v = 0xFF;
        }
        l->b = v;
        GsSortLine(l, &g_ot[g_ot_index], OT_BACK);
        l++;
    }
}

void EffectWaveSlow(void)
{
    u_char i;
    int    x;

    for (i = 0; i < WAVE_STRIPS; i++) {
        g_wave_strips[i].u = ((u_int)g_effect_tick >> 2) & 0x7F;
        x = rcos(((g_effect_tick & 0x1FF) + i * 16) * 8);
        x = x / (rsin(((i + g_effect_tick) & 0xFF) * 16) / 100 + 800);
        g_wave_bank_rows[i].x = x;
        g_wave_bank_rows[i + WAVE_STRIPS].x = x + 0x80;
        g_wave_bank_rows[i + WAVE_STRIPS * 2].x = x + 0x100;
    }
}

void EffectWaveFast(void)
{
    u_char i;
    int    x;

    for (i = 0; i < WAVE_STRIPS; i++) {
        g_wave_strips[i].u = ((u_int)g_effect_tick >> 1) & 0x7F;
        x = rcos(((g_effect_tick & 0x1FF) + i * 16) * 8);
        x = x / (rsin(((i + g_effect_tick) & 0xFF) * 16) / 100 + 220);
        g_wave_bank_rows[i].x = x;
        g_wave_bank_rows[i + WAVE_STRIPS].x = x + 0x80;
        g_wave_bank_rows[i + WAVE_STRIPS * 2].x = x + 0x100;
    }
}
