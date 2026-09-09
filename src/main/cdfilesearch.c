/* Persona 1 (JP) - resolving a path to a CD position.
 *   SLPS_005.00 @ 0x80012D28
 *
 * A unit of its own, well past the readers that call it. The rest of the CD
 * file layer is in the cdfile*.c beside this one, each a unit of its own for
 * the same reason: routines nobody has worked out sit between them.
 */
#include <decomp/types.h>
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
