#ifndef PERSONA_MAIN_CD_H
#define PERSONA_MAIN_CD_H

/* Persona 1 (JP) - SLPS_005.00 CD layer.
 *
 * Holds what more than one of src/p1-jp/main/{cdfile,cdqueue,overlay,preload,
 * psexe}.c needs. Anything only one of them touches - the prebuilt name index
 * in cdfile.c, the streaming-request record in cdqueue.c - stays in that source
 * with the evidence for it.
 */
#include <libcd.h>

typedef struct {
    /* 0x00 */ const char *name;
    /* 0x04 */ void       *dest;
    /* 0x08 */ u_char      mode;      /* bit 7 selects the streaming path */
    /* 0x09 */ u_char      pad[3];
    /* 0x0C */ CdlLOC      loc;
    /* 0x10 */ u_long      size;      /* unsigned: the sector round-up is srl */
    /* 0x14 */ u_char      reserved[0x10];
} CdRequest;                          /* 0x24 bytes */

/* All of these cross the callback boundary, so reads must not be cached: the
   queue is filled by the caller and consumed from CD interrupt context, and the
   three counters are written from both sides. */
extern volatile int       g_cd_busy;        /* 0x80055C10 - spun on before every CD op */
extern volatile int       g_cd_queue_index;
extern volatile int       g_cd_queue_count;
extern volatile CdRequest g_cd_queue[];

/* Defined in cdfile.c. */
/* Game wrapper around CdSearchFile that retries until the file resolves. */
extern CdlFILE *CdSearchFileLoc(CdlFILE *fp, const char *name);
extern void     CdReadToAddr(int size, u_long *dest);
extern int      CdReadPolled(int size, u_long *dest, int mode);
extern void     LoadFileToAddrAsync(const char *name, void *dest);

/* Defined in cdqueue.c. */
extern void CdQueueSubmit(int count);
extern void CdQueueSubmitResolved(int count);

#endif
