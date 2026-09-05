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
    /* 0x10 */ u_long      size;      /* unsigned: the sector round-up is srl */
    /* 0x14 */ u_char      reserved[0x10];
} CdRequest;                          /* 0x24 bytes */

/* All of these cross the callback boundary, so reads must not be cached: the
   queue is filled by the caller and consumed from CD interrupt context, and the
   three counters are written from both sides. */
extern volatile int       g_cd_busy;
extern volatile int       g_cd_queue_index;
extern volatile int       g_cd_queue_count;
extern volatile CdRequest g_cd_queue[];

/* Only ever written, by CdQueueDispatch. Nothing on the disc reads either one -
   not the overlays, not any of the four sub-EXEs - so they are a record of the
   streaming request in flight and nothing more. Being volatile is what kept
   the stores from being optimised out of a build that never reads them back. */
extern volatile u_short g_cd_stream_mode;    /* mode, streaming flag stripped */
extern volatile int     g_cd_stream_sectors; /* size rounded up to sectors    */

extern CdlFILE *CdSearchFileLoc(CdlFILE *fp, const char *name);
extern void     CdReadToAddr(int size, u_long *dest);

/* CdQueueDispatch and CdQueueNextCallback call each other: one request's
   completion starts the next. */
void CdQueueDispatch(void);

/* Clears whichever callback the current request installed.
   Written as a switch, not an if/else: gcc 2.6 expands the two-case chain into
   `beqz .plain / beqz .out / j .stream`, keeping the second test that an
   if/else collapses away. */
void CdQueueClearCallback(void)
{
    CdlFILE unused;
    int     index;

    index = g_cd_queue_index;
    switch (g_cd_queue[index].mode & 0x80) {
    case 0x00:
        CdReadCallback((CdlCB)0);
        break;
    case 0x80:
        CdReadyCallback((CdlCB)0);
        break;
    }
}

/* Data-ready callback for the streaming path. Only a disk error is acted on;
   a normal completion reports whatever state the drive is already in.
   The duplicated `return -3` is deliberate: it lets gcc keep the stored value
   in $v0 and fall straight into the epilogue instead of rematerialising it. */
int CdQueueReadyCallback(u_char status)
{
    CdlLOC unused[3];

    if (status == CdlComplete) {
        return g_cd_busy;
    }
    if (status == CdlDiskError) {
        g_cd_busy = -3;
        return -3;
    }
    return -3;
}

/* Read-done callback for the normal path: advances to the next request, or
   parks the drive at idle once the queue is drained. g_cd_busy == -2 means the
   queue was abandoned while this read was in flight, so no advance happens. */
void CdQueueNextCallback(u_char status)
{
    if (status == CdlComplete) {
        CdQueueClearCallback();
        if (g_cd_busy != -2) {
            g_cd_queue_index = g_cd_queue_index + 1;
            if (g_cd_queue_index < g_cd_queue_count) {
                CdQueueDispatch();
                return;
            }
        }
        g_cd_busy = -1;
    } else if (status == CdlDiskError) {
        g_cd_busy = -3;
    }
}

/* Starts request g_cd_queue_index. A plain read is issued and polled here, then
   handed to CdQueueNextCallback; a streaming request only arms CdlReadN and
   lets the per-sector ready interrupt drive it.
   The switch is what keeps gcc from folding the second mode test away - see
   CdQueueClearCallback. */
void CdQueueDispatch(void)
{
    u_char mode[4];
    int    res;

    switch (g_cd_queue[g_cd_queue_index].mode & 0x80) {
    case 0x00:
        do {
            while (!CdControlB(CdlSetloc,
                               (u_char *)&g_cd_queue[g_cd_queue_index].loc,
                               (u_char *)0))
                ;
            CdReadToAddr(g_cd_queue[g_cd_queue_index].size,
                         g_cd_queue[g_cd_queue_index].dest);
            res = CdReadSync(1, (u_char *)0);
        } while (res == -1);
        CdReadCallback(CdQueueNextCallback);
        break;
    case 0x80:
        g_cd_stream_mode = g_cd_queue[g_cd_queue_index].mode & 0x7F;
        g_cd_stream_sectors =
            (g_cd_queue[g_cd_queue_index].size + 0x7FF) >> 11;
        mode[0] = CdlModeSpeed;

        while (!CdControlB(CdlSetmode, mode, (u_char *)0))
            ;
        CdReadyCallback(CdQueueReadyCallback);
        while (!CdControlB(CdlReadN,
                           (u_char *)&g_cd_queue[g_cd_queue_index].loc,
                           (u_char *)0))
            ;
        break;
    }
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
