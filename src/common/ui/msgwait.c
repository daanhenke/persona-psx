/* Persona 1 (JP) - waiting for a button under a message.
 *
 * Compiled into three overlays rather than called across the boundary:
 *                  DNG         ADV         S2D
 *   MsgWaitPress   0x80085A2C  0x80076EB0  0x80075E68
 *
 * Runs frames until a button goes down, stepping the message each frame when
 * asked to, then gives it one more frame. Defined old-style: the flag comes
 * in as an int and is narrowed here. ADV's fusion screen is the one caller;
 * DNG and S2D carry it unused. ADV's facility screens have their own
 * unconditional form, MsgStepUntilPress.
 */
#include <decomp/types.h>

extern int  g_pad_pressed[];

extern void RunFrame(void);
/* The overlay's message stepper. */
extern int  MsgStep(void);

void MsgWaitPress(step)
    short step;
{
    do {
        if (step) {
            MsgStep();
        }
        RunFrame();
    } while (g_pad_pressed[0] == 0);
    if (step) {
        MsgStep();
    }
    RunFrame();
}
