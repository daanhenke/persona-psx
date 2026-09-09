/* Persona 1 (JP) - a whole file to memory, and a resolved read started.
 *   SLPS_005.00 @ 0x80011EDC LoadFileToAddr, 0x80011F7C CdReadFileToAddrAsync
 *
 * A unit of its own; see cdfilesearch.c.
 */
#include <decomp/types.h>
#include <libcd.h>
#include <persona/main/cd.h>

/* Reads a whole file from the CD to dest, retrying the seek and the read until
   the drive reports a clean transfer. */
void LoadFileToAddr(const char *name, void *dest)
{
    CdlFILE file;
    int     res;

    while (g_cd_busy != -1)
        ;

    CdSearchFileLoc(&file, name);

    do {
        while (!CdControlB(CdlSetloc, (u_char *)&file, (u_char *)0))
            ;
        CdReadToAddr(file.size, dest);
        res = CdReadSync(0, (u_char *)0);
    } while (res == -1);
}
/* Non-blocking counterpart of CdReadFileToAddr. */
void CdReadFileToAddrAsync(CdlFILE *file, int sectors, u_long *dest)
{
    int res;

    while (g_cd_busy != -1)
        ;
    g_cd_busy = 0;

    do {
        while (!CdControlB(CdlSetloc, (u_char *)file, (u_char *)0))
            ;
        while (!CdRead(sectors, dest, 0x80))
            ;
        res = CdReadSync(1, (u_char *)0);
    } while (res == -1);

    CdReadCallback(CdReadDoneCallback);
}
