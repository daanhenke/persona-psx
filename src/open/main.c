/* cc1flags: -O0 -G8 */
/* Persona 1 (JP) - OPEN.EXE, the opening @ 0x80080EF4
 *
 * Plays the opening movie, then runs the title until it asks to leave for
 * the game. Built without optimisation like the rest of this executable.
 */
#include <decomp/types.h>
#include <kernel.h>
#include <libcd.h>
#include <libetc.h>
#include <libgte.h>
#include <libgpu.h>
#include <persona/open/open.h>

int main(void)
{
    ResetCallback();
    func_80086D14();
    func_80086F34();
    SetDispMask(0);
    while (CdInit() == 0) {
    }
    ResetGraph(0);
    PadInit(0);
    InitCARD(1);
    StartCARD();
    _bu_init();
    while (1) {
        if (func_80081018(OpenPlayMovie()) == -1) {
            break;
        }
    }
    SetDispMask(0);
    DrawSync(0);
    func_80086E7C();
    PadStop();
    ResetGraph(0);
    StopCallback();
}
