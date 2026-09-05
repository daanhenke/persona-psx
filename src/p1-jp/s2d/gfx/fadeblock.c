/* Persona 1 (JP) - fades that run to completion before returning.
 *
 * S2D's copy of src/p1-jp/common/gfx/fadeblock.c; its fade-state flag sits
 * 0x20000 higher, which is the only difference.
 *   S2D @ 0x8007FE54 / 0x8007FEDC
 *
 * Two levels move together: the overlay's own six sprites (FadeSpritesStep)
 * and the global level the slot renderer clamps against (FadeStepUp). The
 * sprite step is the one whose "reached the limit" return ends the loop.
 */
#include <types.h>

extern u_char g_fade_state_s2d[];

/* fade.c defines FadeStepUp/Down with u_char parameters; this translation unit
   was built against short ones, so the prototypes disagree on purpose. */
extern int  FadeSpritesStep(short step, short limit);
extern int  FadeSpritesStepDown(short step, short floor);
extern int  FadeStepUp(short step, short limit);
extern int  FadeStepDown(short step, short floor);
extern void RunFrame(void);

/* Brightens by `step` a frame until the sprites reach `limit`, drawing a frame
   between each step, and does not return until the fade has finished. Fade
   state 2 marks a fade-up in progress; the loop leaves it at 0.
 *
 * The tail is spelled out a second time because the loop exits the moment the
 * sprite step reports it has reached the limit, and that final step still has
 * to be applied to the global level and drawn. */
void FadeUpBlocking(short step, short limit)
{
    g_fade_state_s2d[0] = 2;
    for (;;) {
        if (FadeSpritesStep(step, limit)) {
            break;
        }
        FadeStepUp(step, limit);
        RunFrame();
    }
    FadeStepUp(step, limit);
    RunFrame();
    g_fade_state_s2d[0] = 0;
}

/* The same, darkening to `floor`. Fade state 1 marks a fade-down. */
void FadeDownBlocking(short step, short floor)
{
    g_fade_state_s2d[0] = 1;
    for (;;) {
        if (FadeSpritesStepDown(step, floor)) {
            break;
        }
        FadeStepDown(step, floor);
        RunFrame();
    }
    FadeStepDown(step, floor);
    RunFrame();
    g_fade_state_s2d[0] = 0;
}
