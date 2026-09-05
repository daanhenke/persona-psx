/* Persona 1 (JP) - fades that run to completion before returning.
 *
 * S2D's copy of src/p1-jp/common/fadeblock.c; its fade-state flag sits
 * 0x20000 higher, which is the only difference.
 *   S2D @ 0x8007FE54 / 0x8007FEDC
 *
 * Two levels move together: the overlay's own six sprites (FadeSpritesStep)
 * and the global level the slot renderer clamps against (FadeStepUp). The
 * sprite step is the one whose "reached the limit" return ends the loop.
 */
#include <types.h>

extern u_char g_fade_state_s2d[];

/* FadeStepUp/Down are defined in fade.c against u_char parameters. This
   translation unit was built against short ones - the target passes the same
   sign-extended registers to them as to FadeSpritesStep, with none of the
   `andi 0xff` masking a u_char prototype would produce. */
extern int  FadeSpritesStep(short step, short limit);
extern int  FadeSpritesStepDown(short step, short floor);
extern int  FadeStepUp(short step, short limit);
extern int  FadeStepDown(short step, short floor);
extern void RunFrame(void);

/* The final step is written out a second time rather than folded into the
   loop: the loop exits the moment FadeSpritesStep reports it has reached the
   limit, and that last step still has to be applied to the global level and
   drawn. Restructuring it into one copy inside the loop changes the branch
   shape - the original really does have the tail twice. */
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
