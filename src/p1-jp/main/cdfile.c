/* Persona 1 (JP) - CD file loading.  SLPS_005.00 @ 0x80011E3C */
#include <libcd.h>
#include <persona/main/cd.h>

/* Prebuilt CD index shipped as FNAME/FSECT/FSIZE.DAT and loaded at boot, so the
   game never pays for an ISO9660 directory search. */
extern const char g_cd_filenames[];   /* packed NUL-terminated names */
extern int        g_cd_file_lba[];
extern int        g_cd_file_size[];

extern int  strcmp(const char *a, const char *b);

/* Resolves a path to a CD position by its index in the shipped name list, and
   returns the file it filled in - same shape as the SDK's CdSearchFile. */
CdlFILE *CdSearchFileLoc(CdlFILE *file, const char *name)
{
    const char *p;
    int         index;

    p = g_cd_filenames;
    index = 0;
    while (strcmp(p, name) != 0) {
        while (*p != '\0') {
            p++;
        }
        p++;
        index++;
    }

    CdIntToPos(g_cd_file_lba[index], &file->pos);
    file->size = g_cd_file_size[index];
    return file;
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

/* Completion callback for the async path: clears itself and marks the drive
   idle on success, or records the error. Any other status is ignored. */
void CdReadDoneCallback(u_char status)
{
    if (status == CdlComplete) {
        CdReadCallback((CdlCB)0);
        g_cd_busy = -1;
    } else if (status == CdlDiskError) {
        g_cd_busy = -3;
    }
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

/* Issues a single read and polls it to completion, pumping VSync while it
   waits. Returns the final CdReadSync status. */
int CdReadPolled(int size, u_long *dest, int mode)
{
    int res;

    CdRead((size + 0x7FF) / 2048, dest, mode);
    while ((res = CdReadSync(1, (u_char *)0)) > 0) {
        VSync(0);
    }
    return res;
}

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
