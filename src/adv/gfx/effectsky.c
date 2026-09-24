/* Persona 1 (JP) - the blue sky with drifting sparkles.  ADV only.
 *   ADV 0x800871DC
 *
 * One frame of a full-screen effect. The sky is 240 one-pixel lines across
 * the screen, blue brightening towards the middle row and falling off again
 * below it, all scaled by the screen's fade level (g_bg_layers[0].r).
 *
 * Over it sixteen sparkles, played as the six frames of g_effect3_frames, are
 * moved in place: each keeps a home position and drifts across on a sine of
 * the effect tick, wrapping at the screen's edges, while falling down it at
 * a speed of its own. The sixteen are four groups of four, each group with its
 * own sway and fall rate, and every frame of the six gets the same position.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <persona/adv/effect.h>
#include <persona/adv/ot.h>
#include <persona/common/bg.h>

#define SCREEN_W  320
#define SKY_ROWS  240
#define SKY_MID   120
#define SKY_PRI   0x400

/* 240 lines, reached by hardcoded address. */
#define g_sky_lines ((GsGLINE *)(0x800ED180 + WORK_BIAS))

typedef struct {
    u_short x;
    u_short y;
} Home;

#define WAVE(group, sway0, fall0)                                                                                                 \
    for (i = 0; i < 4; i++) {                                                                                                     \
        g_effect3_frames[0].spr[(group) * 4 + i].x = ((home[group] + i)->x + rsin(g_effect_tick) / ((sway0) + i * 2)) % SCREEN_W; \
        g_effect3_frames[1].spr[(group) * 4 + i].x = ((home[group] + i)->x + rsin(g_effect_tick) / ((sway0) + i * 2)) % SCREEN_W; \
        g_effect3_frames[2].spr[(group) * 4 + i].x = ((home[group] + i)->x + rsin(g_effect_tick) / ((sway0) + i * 2)) % SCREEN_W; \
        g_effect3_frames[3].spr[(group) * 4 + i].x = ((home[group] + i)->x + rsin(g_effect_tick) / ((sway0) + i * 2)) % SCREEN_W; \
        g_effect3_frames[4].spr[(group) * 4 + i].x = ((home[group] + i)->x + rsin(g_effect_tick) / ((sway0) + i * 2)) % SCREEN_W; \
        g_effect3_frames[5].spr[(group) * 4 + i].x = ((home[group] + i)->x + rsin(g_effect_tick) / ((sway0) + i * 2)) % SCREEN_W; \
        g_effect3_frames[0].spr[(group) * 4 + i].y = ((u_char)(g_effect_tick / (i + (fall0))) + (home[group] + i)->y) & 0xFF;     \
        g_effect3_frames[1].spr[(group) * 4 + i].y = ((u_char)(g_effect_tick / (i + (fall0))) + (home[group] + i)->y) & 0xFF;     \
        g_effect3_frames[2].spr[(group) * 4 + i].y = ((u_char)(g_effect_tick / (i + (fall0))) + (home[group] + i)->y) & 0xFF;     \
        g_effect3_frames[3].spr[(group) * 4 + i].y = ((u_char)(g_effect_tick / (i + (fall0))) + (home[group] + i)->y) & 0xFF;     \
        g_effect3_frames[4].spr[(group) * 4 + i].y = ((u_char)(g_effect_tick / (i + (fall0))) + (home[group] + i)->y) & 0xFF;     \
        g_effect3_frames[5].spr[(group) * 4 + i].y = ((u_char)(g_effect_tick / (i + (fall0))) + (home[group] + i)->y) & 0xFF;     \
    }

#ifdef NON_MATCHING
void EffectSkyStep(void)
{
    GsGLINE *line = g_sky_lines;
    Home home[4][4] = {
        { { 0x20, 0x20 }, { 0x60, 0x80 }, { 0x40, 0xA0 }, { 0xF0, 0xE0 } },
        { { 0x30, 0x30 }, { 0x70, 0x80 }, { 0xA0, 0x40 }, { 0xE0, 0x10 } },
        { { 0x10, 0xF0 }, { 0xA0, 0x38 }, { 0x60, 0x40 }, { 0xC0, 0xB0 } },
        { { 0x00, 0x00 }, { 0xE0, 0x30 }, { 0xC0, 0x98 }, { 0x100, 0x80 } },
    };
    int      i;
    int      sway;
    int      shade;

    for (i = 0; i < SKY_MID; i++) {
        line->attribute = 0;
        line->x0 = 0;
        line->x1 = SCREEN_W;
        line->y0 = i;
        line->y1 = i;
        shade = (i + 8) * g_bg_layers[0].r / 128;
        line->r0 = line->r1 = 0;
        line->g0 = line->g1 = 0;
        line->b0 = line->b1 = shade;
        GsSortGLine(line, &g_ot[g_ot_index], SKY_PRI);
        line++;
    }
    for (i = SKY_MID; i < SKY_ROWS; i++) {
        line->attribute = 0;
        line->x0 = 0;
        line->x1 = SCREEN_W;
        line->y0 = i;
        line->y1 = i;
        shade = (0xF8 - i) * g_bg_layers[0].r / 128;
        line->r0 = line->r1 = 0;
        line->g0 = line->g1 = 0;
        line->b0 = line->b1 = shade;
        GsSortGLine(line, &g_ot[g_ot_index], SKY_PRI);
        line++;
    }

    WAVE(0, 0x32, 8);
    WAVE(1, 0x14, 12);
    WAVE(2, 0x1E, 6);
    WAVE(3, 0x28, 4);
}
#else
INCLUDE_ASM("adv/nonmatchings/gfx/effectsky", EffectSkyStep);
#endif
