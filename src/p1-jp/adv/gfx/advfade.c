/* Persona 1 (JP) - the ADV overlay's own blocking fades.
 *   0x80085DC0 AdvFadeUpBlocking   0x80085EA0 AdvFadeDownBlocking
 *
 * The same shape as the pair in common/gfx/fadeblock.c, and for the same
 * reason: two levels move together, and the sprite step is the one whose
 * "reached the limit" return ends the loop, so the tail is spelled out again
 * for the step that ended it.
 *
 * What differs is the frame in between. These spin on AdvRunFrame, and while
 * the ADVCHR sequence is running they draw the character layer on its own
 * instead - nothing else is on screen for them to draw.
 */
#include <types.h>

extern u_char g_fade_state[];
extern u_char g_adv_char_seq;

/* The real prototypes: the shared copy of these fades was built against
   short-taking ones, and this unit was not. */
extern int  FadeSpritesStep(short step, short limit);
extern int  FadeSpritesStepDown(short step, short floor);
extern void FadeStepUp(u_char step, u_char limit);
extern void FadeStepDown(u_char step, u_char floor);

extern void AdvRunFrame(void);
extern void AdvRenderCharFrame(short extra);

/* Fade state 2 marks a fade-up in progress; the loop leaves it at 0. */
void AdvFadeUpBlocking(short step, short limit)
{
    g_fade_state[0] = 2;
    for (;;) {
        if (FadeSpritesStep(step, limit)) {
            break;
        }
        FadeStepUp(step, limit);
        if (g_adv_char_seq) {
            AdvRenderCharFrame(0);
        } else {
            AdvRunFrame();
        }
    }
    FadeStepUp(step, limit);
    if (g_adv_char_seq) {
        AdvRenderCharFrame(0);
    } else {
        AdvRunFrame();
    }
    g_fade_state[0] = 0;
}

/* The same, darkening to `floor`. Fade state 1 marks a fade-down. */
void AdvFadeDownBlocking(short step, short floor)
{
    g_fade_state[0] = 1;
    for (;;) {
        if (FadeSpritesStepDown(step, floor)) {
            break;
        }
        FadeStepDown(step, floor);
        if (g_adv_char_seq) {
            AdvRenderCharFrame(0);
        } else {
            AdvRunFrame();
        }
    }
    FadeStepDown(step, floor);
    if (g_adv_char_seq) {
        AdvRenderCharFrame(0);
    } else {
        AdvRunFrame();
    }
    g_fade_state[0] = 0;
}
