/* Persona 1 (JP) - sub-EXE launcher.  SLPS_005.00 @ 0x80011864
 *
 * ATLUS.EXE, OPEN.EXE, MOVIE.EXE and END.EXE are separate PS-EXEs run through
 * the BIOS. Control returns here afterwards, so everything the sub-EXE tore
 * down has to be brought back up.
 */
#include <types.h>
#include <libapi.h>
#include <libcd.h>
#include <libetc.h>
#include <libgpu.h>
#include <libsnd.h>
#include <persona/main/psexe.h>

/* The sub-EXE runs on its own stack, just below the top of RAM. */
#define SUBEXE_STACK 0x801FBFF0

void LoadAndExecPsExe(const char *name)
{
    struct EXEC exec;

    while (CdLoadPsExe(name, &exec) != 0)
        ;

    DrawSync(0);
    ResetGraph(0);
    PadStop();
    StopCallback();

    exec.s_addr = SUBEXE_STACK;
    exec.s_size = 0;

    EnterCriticalSection();
    Exec(&exec, 1, 0);

    ResetCallback();
    while (!CdInit())
        ;
    PadInit(0);
    SsEnd();
    SsQuit();
    SsInit();
}
