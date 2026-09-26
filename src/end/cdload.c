/* Persona 1 (JP) - END.EXE - CD file loads.  @ 0x80082DD4
 *
 * An earlier build of the resident's loaders (src/main/cdfile*.c): each one
 * spins until the drive is idle, then reads a whole file, by name or from a
 * position, either waiting for it or leaving CdReadDoneCallback to report
 * completion through g_cd_busy.
 */
#include <decomp/types.h>
#include <libcd.h>
#include <persona/end/end.h>

/* Defined here: the unit reaches it gp-relative. -1 when the drive is idle,
   0 while an asynchronous read is under way. */
volatile int g_cd_busy;

void CdReadToAddr(int size, u_long *dest);
void CdReadDoneCallback(void);

/* Reads the file `name` to `dest` and returns at once; the callback marks
   the drive idle again. */
void LoadFileToAddrAsync(char *name, u_long *dest)
{
    CdlFILE file;
    int     res;

    while (g_cd_busy != -1) {
    }
    g_cd_busy = 0;

    while (!CdSearchFile(&file, name)) {
    }

    do {
        while (!CdControlB(CdlSetloc, (u_char *)&file, 0)) {
        }
        CdReadToAddr(file.size, dest);
        res = CdReadSync(1, 0);
    } while (res == -1);

    CdReadCallback((CdlCB)CdReadDoneCallback);
}

/* Reads the file `name` to `dest` and waits for it. */
void LoadFileToAddr(char *name, u_long *dest)
{
    CdlFILE file;
    int     res;

    while (g_cd_busy != -1) {
    }

    while (!CdSearchFile(&file, name)) {
    }

    do {
        while (!CdControlB(CdlSetloc, (u_char *)&file, 0)) {
        }
        CdReadToAddr(file.size, dest);
        res = CdReadSync(0, 0);
    } while (res == -1);
}

/* Reads `sectors` from `pos` to `dest` and returns at once; the callback
   marks the drive idle again. */
void CdReadFileToAddrAsync(CdlLOC *pos, int sectors, u_long *dest)
{
    int res;

    while (g_cd_busy != -1) {
    }
    g_cd_busy = 0;

    do {
        while (!CdControlB(CdlSetloc, (u_char *)pos, 0)) {
        }
        while (!CdRead(sectors, dest, CdlModeSpeed)) {
        }
        res = CdReadSync(1, 0);
    } while (res == -1);

    CdReadCallback((CdlCB)CdReadDoneCallback);
}

/* Reads `sectors` from `pos` to `dest` and waits for it. */
void CdReadFileToAddr(CdlLOC *pos, int sectors, u_long *dest)
{
    int res;

    while (g_cd_busy != -1) {
    }

    do {
        while (!CdControlB(CdlSetloc, (u_char *)pos, 0)) {
        }
        while (!CdRead(sectors, dest, CdlModeSpeed)) {
        }
        res = CdReadSync(0, 0);
    } while (res == -1);
}

/* Starts reading `size` bytes, rounded up to whole sectors, to `dest`. */
void CdReadToAddr(int size, u_long *dest)
{
    int sectors;

    sectors = (size + 0x7FF) / 2048;
    while (!CdRead(sectors, dest, CdlModeSpeed)) {
    }
}

/* The end of an asynchronous read: nothing more to report. */
void CdReadDoneCallback(void)
{
    CdReadCallback((CdlCB)0);
    g_cd_busy = -1;
}
