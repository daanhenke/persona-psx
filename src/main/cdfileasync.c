/* Persona 1 (JP) - a whole file to memory without waiting for it.
 *   SLPS_005.00 @ 0x80011C18
 *
 * Kicks the read off and lets CdReadDoneCallback report completion through
 * g_cd_busy. A unit of its own; see cdfilesearch.c.
 */
#include <decomp/types.h>
#include <libcd.h>
#include <persona/main/cd.h>

/* Non-blocking counterpart of LoadFileToAddr: kicks the read off and lets
   CdReadDoneCallback report completion through g_cd_busy. */
void LoadFileToAddrAsync(const char *name, void *dest)
{
    CdlFILE file;
    int     res;

    while (g_cd_busy != -1)
        ;
    g_cd_busy = 0;

    CdSearchFileLoc(&file, name);

    do {
        while (!CdControlB(CdlSetloc, (u_char *)&file, (u_char *)0))
            ;
        CdReadToAddr(file.size, dest);
        res = CdReadSync(1, (u_char *)0);
    } while (res == -1);

    CdReadCallback(CdReadDoneCallback);
}
