/* Persona 1 (JP) - the battle overlay's blocking and async sector reads.
 *   BTLP @ 0x80091D2C BtlReadSectors, 0x80091DA8 BtlReadSectorsAsync
 *
 * The blocking read draws a frame every time round the wait, so an animation
 * already on screen keeps running while the next piece loads.
 *
 * The pack-entry callers sit elsewhere in the image and live in their own
 * units: packopen.c, seekpackentry.c and seekfile.c.
 */
#include <decomp/types.h>
#include <libcd.h>
#include <persona/main/cd.h>
#include <persona/btlp/battle.h>

extern void CdReadFileToAddrAsync(CdlFILE *file, int sectors, u_long *dest);

void BtlReadSectors(u_long *dest, int sector, int sectors)
{
    CdlLOC loc;

    CdIntToPos(sector, &loc);
    CdReadFileToAddrAsync((CdlFILE *)&loc, sectors, dest);
    while (g_cd_busy != -1) {
        BtlDrawFrame();
    }
}

void BtlReadSectorsAsync(u_long *dest, int sector, int sectors)
{
    CdlLOC loc;

    CdIntToPos(sector, &loc);
    CdReadFileToAddrAsync((CdlFILE *)&loc, sectors, dest);
}
