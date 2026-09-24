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

/* One group of four sparkles: each drifts on a sine of the tick, divided by a
   sway that widens by two a sparkle, and falls at the tick divided by its own
   rate, the byte it lands on wrapping round the screen's height. */
#define WAVE(group, sway0, fall0)                                              \
    for (i = 0; i < 4; i++) {                                                   \
        g_effect3_frames[0].spr[(group) * 4 + i].x =                            \
            ((home[group] + i)->x + rsin(g_effect_tick) / ((sway0) + i * 2))    \
            % SCREEN_W;                                                         \
        g_effect3_frames[1].spr[(group) * 4 + i].x =                            \
            ((home[group] + i)->x + rsin(g_effect_tick) / ((sway0) + i * 2))    \
            % SCREEN_W;                                                         \
        g_effect3_frames[2].spr[(group) * 4 + i].x =                            \
            ((home[group] + i)->x + rsin(g_effect_tick) / ((sway0) + i * 2))    \
            % SCREEN_W;                                                         \
        g_effect3_frames[3].spr[(group) * 4 + i].x =                            \
            ((home[group] + i)->x + rsin(g_effect_tick) / ((sway0) + i * 2))    \
            % SCREEN_W;                                                         \
        g_effect3_frames[4].spr[(group) * 4 + i].x =                            \
            ((home[group] + i)->x + rsin(g_effect_tick) / ((sway0) + i * 2))    \
            % SCREEN_W;                                                         \
        g_effect3_frames[5].spr[(group) * 4 + i].x =                            \
            ((home[group] + i)->x + rsin(g_effect_tick) / ((sway0) + i * 2))    \
            % SCREEN_W;                                                         \
        fall = (u_char)(g_effect_tick / (i + (fall0)));                         \
        g_effect3_frames[0].spr[(group) * 4 + i].y =                            \
            ((home[group] + i)->y + fall) & 0xFF;                               \
        g_effect3_frames[1].spr[(group) * 4 + i].y =                            \
            ((home[group] + i)->y + fall) & 0xFF;                               \
        g_effect3_frames[2].spr[(group) * 4 + i].y =                            \
            ((home[group] + i)->y + fall) & 0xFF;                               \
        g_effect3_frames[3].spr[(group) * 4 + i].y =                            \
            ((home[group] + i)->y + fall) & 0xFF;                               \
        g_effect3_frames[4].spr[(group) * 4 + i].y =                            \
            ((home[group] + i)->y + fall) & 0xFF;                               \
        g_effect3_frames[5].spr[(group) * 4 + i].y =                            \
            ((home[group] + i)->y + fall) & 0xFF;                               \
    }

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
    /* Never used, and n is only a copy of an index the other groups write
       out in full: both are what gets the image's register assignment out
       of the allocator (found with the permuter), not anything the routine
       needs. Without either, the entry offset and the sway trade s1 and s2
       in three of the four groups. */
    int      sway;
    int      fall;
    int      n;
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
    for (i = 0; i < 4; i++) {
        g_effect3_frames[0].spr[2 * 4 + i].x =
            ((home[2] + i)->x + rsin(g_effect_tick) / (0x1E + i * 2)) % SCREEN_W;
        g_effect3_frames[1].spr[2 * 4 + i].x =
            ((home[2] + i)->x + rsin(g_effect_tick) / (0x1E + i * 2)) % SCREEN_W;
        g_effect3_frames[2].spr[2 * 4 + i].x =
            ((home[2] + i)->x + rsin(g_effect_tick) / (0x1E + i * 2)) % SCREEN_W;
        g_effect3_frames[3].spr[2 * 4 + i].x =
            ((home[2] + i)->x + rsin(g_effect_tick) / (0x1E + i * 2)) % SCREEN_W;
        g_effect3_frames[4].spr[2 * 4 + i].x =
            ((home[2] + i)->x + rsin(g_effect_tick) / (0x1E + i * 2)) % SCREEN_W;
        g_effect3_frames[5].spr[2 * 4 + i].x =
            ((home[2] + i)->x + rsin(g_effect_tick) / (0x1E + i * 2)) % SCREEN_W;
        fall = (u_char)(g_effect_tick / (i + 6));
        n = 2 * 4 + i;
        g_effect3_frames[0].spr[n].y = ((home[2] + i)->y + fall) & 0xFF;
        g_effect3_frames[1].spr[n].y = ((home[2] + i)->y + fall) & 0xFF;
        g_effect3_frames[2].spr[n].y = ((home[2] + i)->y + fall) & 0xFF;
        g_effect3_frames[3].spr[n].y = ((home[2] + i)->y + fall) & 0xFF;
        g_effect3_frames[4].spr[2 * 4 + i].y = ((home[2] + i)->y + fall) & 0xFF;
        g_effect3_frames[5].spr[n].y = ((home[2] + i)->y + fall) & 0xFF;
    }
    WAVE(3, 0x28, 4);
}
