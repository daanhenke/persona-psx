/* Persona 1 (JP) - PS-EXE loader.  SLPS_005.00 @ 0x80012BEC
 *
 * The BIOS Load() would do this, but it goes through the ISO9660 directory;
 * the game resolves the name through its own prebuilt CD index instead, so it
 * has to unpack the PS-EXE header itself. The header lives at file offset 0x10
 * and is exactly a struct EXEC; the text image starts in the next sector.
 */
#include <types.h>
#include <libapi.h>
#include <libcd.h>
#include <persona/main/cd.h>
#include <persona/main/psexe.h>

#define PSEXE_HEADER_OFF 0x10   /* struct EXEC's position inside sector 0 */
#define PSEXE_TRIES      10

/* Fills in *exec and loads the text image. Returns 0 once a pass gets all the
   way through, -1 after ten failed attempts - the caller spins on it, so a
   scratched disc keeps retrying rather than hanging inside one read. */
int CdLoadPsExe(const char *name, struct EXEC *exec)
{
    CdlFILE file;
    u_char  sector[2048];
    int     tries;
    int     mode;

    mode = CdlModeSpeed;        /* double speed, for both reads */

    for (tries = 0; tries < PSEXE_TRIES; tries++) {
        if (CdSearchFileLoc(&file, name) == (CdlFILE *)0) {
            continue;
        }

        while (!CdControlB(CdlSetloc, (u_char *)&file, (u_char *)0))
            ;
        if (CdReadPolled(sizeof(sector), (u_long *)sector, mode) != 0) {
            continue;
        }

        *exec = *(struct EXEC *)(sector + PSEXE_HEADER_OFF);

        /* Step past the header sector to where the text image starts. */
        CdIntToPos(CdPosToInt(&file) + 1, &file.pos);
        while (!CdControlB(CdlSetloc, (u_char *)&file, (u_char *)0))
            ;
        if (CdReadPolled(exec->t_size, (u_long *)exec->t_addr, mode) == 0) {
            return 0;
        }
    }

    return -1;
}
