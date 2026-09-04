/* Persona 1 (JP) - CD request queue.  SLPS_005.00 @ 0x80011CFC, 0x80011DD0
 *
 * Callers fill in g_cd_queue and submit a count; CdQueueDispatch then starts
 * request g_cd_queue_index, either as a plain read or, when the mode byte has
 * bit 7 set, as a streaming read.
 */
#include <libcd.h>

typedef struct {
    /* 0x00 */ const char *name;
    /* 0x04 */ void       *dest;
    /* 0x08 */ u_char      mode;      /* bit 7 selects the streaming path */
    /* 0x09 */ u_char      pad[3];
    /* 0x0C */ CdlLOC      loc;
    /* 0x10 */ int         size;
    /* 0x14 */ u_char      reserved[0x10];
} CdRequest;                          /* 0x24 bytes */

/* All three are updated from the CD callbacks, so reads must not be cached. */
extern volatile int g_cd_busy;
extern volatile int g_cd_queue_index;
extern volatile int g_cd_queue_count;
extern CdRequest    g_cd_queue[];

extern CdlFILE *CdSearchFileLoc(CdlFILE *fp, const char *name);
extern void     CdQueueDispatch(void);

/* Clears whichever callback the current request installed. */
void CdQueueClearCallback(void)
{
    CdlFILE unused;
    int     index;

    /* The second test is redundant but present in the original. Copying the
       volatile index to a local lets the mode load be shared while leaving
       both branches standing. */
    index = g_cd_queue_index;
    if ((g_cd_queue[index].mode & 0x80) == 0) {
        CdReadCallback((CdlCB)0);
    } else if ((g_cd_queue[index].mode & 0x80) != 0) {
        CdReadyCallback((CdlCB)0);
    }
}

/* Data-ready callback for the streaming path. Only a disk error is acted on;
   a normal completion leaves g_cd_busy alone. */
int CdQueueReadyCallback(u_char status)
{
    CdlLOC unused[3];
    int    state;

    if (status == CdlComplete) {
        return g_cd_busy;
    }
    state = -3;
    if (status == CdlDiskError) {
        g_cd_busy = state;
    }
    return state;
}

/* Resolves every queued name to a CD position, then starts the first request. */
void CdQueueSubmit(int count)
{
    CdlFILE unused;
    int     i;

    g_cd_queue_index = 0;
    g_cd_queue_count = count;
    if (g_cd_queue_count != 0) {
        while (g_cd_busy != -1)
            ;
        g_cd_busy = 0;

        for (i = 0; i < count; i++) {
            CdSearchFileLoc((CdlFILE *)&g_cd_queue[i].loc, g_cd_queue[i].name);
        }

        CdQueueDispatch();
    }
}

/* Same, for entries whose CdlLOC the caller has already filled in.
   `unused` is not dead weight by accident: the original reserves the same
   0x30 frame as CdQueueSubmit, and gcc 2.6 still allocates declared locals. */
void CdQueueSubmitResolved(int count)
{
    CdlFILE unused;

    g_cd_queue_index = 0;
    g_cd_queue_count = count;
    if (g_cd_queue_count != 0) {
        while (g_cd_busy != -1)
            ;
        g_cd_busy = 0;
        CdQueueDispatch();
    }
}
