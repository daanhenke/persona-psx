/* Persona 1 (JP) - overlay loader.  SLPS_005.00 @ 0x800119DC
 *
 * Every overlay (DNG/BTLP/S2D/ADV/CASINO/NAME) is read to the same base, so
 * only one is resident at a time; the caller passes the g_overlay_table entry
 * naming the file and its entry point.
 */
#include <libcd.h>
#include <persona/main/cd.h>

/* Resident game global. */
extern u_long *g_overlay_dest;      /* 0x8001014C - points at g_overlay_base   */

typedef struct {
    const char *name;
    void      (*entry)(void);
} Overlay;

void LoadOverlay(Overlay *ovl)
{
    CdlFILE file;
    u_long  nsec;
    int     res;

    while (g_cd_busy != -1)
        ;

    CdSearchFileLoc(&file, ovl->name);

    do {
        while (!CdControlB(CdlSetloc, (u_char *)&file, (u_char *)0))
            ;
        nsec = (file.size + 0x7FF) >> 11;
        while (!CdRead(nsec, g_overlay_dest, 0x80))
            ;
        res = CdReadSync(0, (u_char *)0);
    } while (res == -1);

    ovl->entry();
}
