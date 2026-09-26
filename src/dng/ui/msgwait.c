/* Persona 1 (JP) - waiting for a button under a message.  DNG only.
 *   0x80085A2C MsgWaitPress
 *
 * Runs frames until a button goes down, stepping the message each frame when
 * asked to, then gives it one more frame. Defined old-style: the flag comes
 * in as an int and is narrowed here. Like MenuChoicePageOpen, nothing calls
 * it and no JP1 file holds its address.
 */
#include <decomp/types.h>

extern int  g_pad_pressed[];

extern void RunFrame(void);
/* The field's message stepper. */
extern int  func_80076380(void);

void MsgWaitPress(step)
    short step;
{
    do {
        if (step) {
            func_80076380();
        }
        RunFrame();
    } while (g_pad_pressed[0] == 0);
    if (step) {
        func_80076380();
    }
    RunFrame();
}
