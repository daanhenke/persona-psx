/* Persona 1 (JP) - global screen fade level, S2D's copy.
 *
 * The same three as src/p1-jp/common/fade.c (which covers DNG and ADV); S2D's
 * work area sits 0x20000 higher, which is the only difference.
 *   S2D @ 0x80065AFC / 0x80065B34 / 0x80065B6C
 *
 * One byte, 0 (black) to 0x80 (full). The slot renderer reads it every frame
 * and clamps each slot's own brightness against it before writing the result
 * into the GsSPRITE r/g/b, so lowering this darkens everything at once.
 *
 * Callers ramp it from a loop: reset to 0 with FadeStepDown(0, 0), then call
 * FadeStepUp(2, 0x80) once per frame until it returns 1.
 */
#include <types.h>
#include <libgs.h>

/* Literal addresses, not linker symbols - the whole work area is reached by
   hardcoded address here. Both are held in pointer locals below rather than
   folded into each access, which is how the original addresses them. */
#define g_sprites ((GsSPRITE *)0x800FD64C)

/* The 512 sprites the renderer fills in each frame. Blacking out their colour
   as well as the level itself is what makes this a hard cut rather than the
   start of a ramp: the sprites already sorted into this frame's OT keep the
   colour they were given, so clearing the level alone would leave one frame of
   the old image on screen. */
void FadeBlackout(void)
{
    u_char *fade_level;
    int     i;

    fade_level = (u_char *)0x800FC00C;
    for (i = 0; i < 512; i++) {
        g_sprites[i].r = 0;
        g_sprites[i].g = 0;
        g_sprites[i].b = 0;
    }
    *fade_level = 0;
}

/* Raises the level by `step`, stopping at `limit`. Returns 1 on the call that
   hits the limit, which is how the ramp loops know they are done. */
int FadeStepUp(u_char step, u_char limit)
{
    int hit;
    int level;

    hit = 0;
    level = *(u_char *)0x800FC00C + step;
    if (level > limit) {
        level = limit;
        hit = 1;
    }
    *(u_char *)0x800FC00C = level;
    return hit;
}

/* Lowers the level by `step`, stopping at `floor`. Called as (0, 0) to force
   the level to zero without touching the sprites. */
int FadeStepDown(u_char step, u_char floor)
{
    int hit;
    int level;

    hit = 0;
    level = *(u_char *)0x800FC00C - step;
    if (level < floor) {
        level = floor;
        hit = 1;
    }
    *(u_char *)0x800FC00C = level;
    return hit;
}
