/* Persona 1 (JP) - brightness ramp for the overlay's own sprite set.
 *
 * Compiled into three overlays rather than called across the boundary:
 *   DNG @ 0x8008FA54
 *   ADV @ 0x8008B82C
 *   S2D @ 0x8007FF64
 *
 * These six GsSPRITEs live in the overlay's data rather than the work area,
 * and they carry the fade level themselves: the current value is read back out
 * of sprite 0's red channel, not kept in a separate variable. The global
 * FadeStepUp in fade.c is the counterpart for everything the slot renderer
 * draws; callers usually step both together.
 */
#include <types.h>
#include <libgs.h>

extern GsSPRITE g_fade_sprites[];

/* Returns 1 on the call that reaches `limit`, which is how the blocking fade
   loops know to stop. */
int FadeSpritesStep(short step, short limit)
{
    int level;
    int done;
    int i;

    level = g_fade_sprites[0].r + step;
    done = 0;
    if (level > limit) {
        level = limit;
        done = 1;
    }
    for (i = 0; i < 6; i++) {
        g_fade_sprites[i].r = level;
        g_fade_sprites[i].g = level;
        g_fade_sprites[i].b = level;
    }
    return done;
}

/* The downward counterpart, stopping at `floor`. */
int FadeSpritesStepDown(short step, short floor)
{
    int level;
    int done;
    int i;

    level = g_fade_sprites[0].r - step;
    done = 0;
    if (level < floor) {
        level = floor;
        done = 1;
    }
    for (i = 0; i < 6; i++) {
        g_fade_sprites[i].r = level;
        g_fade_sprites[i].g = level;
        g_fade_sprites[i].b = level;
    }
    return done;
}
