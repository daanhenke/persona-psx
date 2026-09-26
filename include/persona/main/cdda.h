#ifndef PERSONA_MAIN_CDDA_H
#define PERSONA_MAIN_CDDA_H

/* Persona 1 (JP) - SLPS_005.00 CD-DA playback, cdstream.c and cddaready.c. */
#include <libcd.h>

/* g_cd_da_repeat: how many more times the track plays. The top bit is set
   once the first pass has ended; CDDA_REPEAT_FOREVER never counts down. */
#define CDDA_REPEAT_FOREVER 0x7FFFFFFF
#define CDDA_REPEAT_PASSED  0x80000000

extern CdlLOC       g_cd_da_start;     /* where the track starts           */
extern volatile int g_cd_da_repeat;
extern volatile CdlLOC g_cd_da_pos;   /* where the head last reported     */
extern CdlLOC       g_cd_da_end_loc;   /* one sector past the track        */
extern int          g_cd_da_end;       /* the same as a sector number      */

extern void CdDaSetFile(const char *name);
extern void CdDaPlay(int repeat);
extern void CdDaPause(void);
extern void CdDaResume(void);
extern void CdDaReadyCallback(u_char status, u_char *result);

#endif
