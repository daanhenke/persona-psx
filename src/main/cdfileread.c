/* Persona 1 (JP) - the two blocking CD reads.
 *   SLPS_005.00 @ 0x80012090 CdReadFileToAddr, 0x80012144 CdReadToAddr
 *
 * A unit of its own; see cdfilesearch.c.
 */
#include <decomp/types.h>
#include <libcd.h>
#include <persona/main/cd.h>

/* Reads `sectors` sectors from an already-resolved CD location, retrying the
   seek and the read until the drive reports a clean transfer. */
void CdReadFileToAddr(CdlFILE *file, int sectors, u_long *dest)
{
    int res;

    while (g_cd_busy != -1)
        ;

    do {
        while (!CdControlB(CdlSetloc, (u_char *)file, (u_char *)0))
            ;
        while (!CdRead(sectors, dest, 0x80))
            ;
        res = CdReadSync(0, (u_char *)0);
    } while (res == -1);
}

/* Rounds a byte count up to whole 2048-byte sectors and reads them from
   wherever the drive was last positioned, retrying until it accepts the
   request. Mode 0x80 is CdlModeSpeed - double speed. */
void CdReadToAddr(int size, u_long *dest)
{
    int sectors;

    sectors = (size + 0x7FF) / 2048;
    while (!CdRead(sectors, dest, 0x80))
        ;
}
