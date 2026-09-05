/* Persona 1 (JP) - CD request queue.  SLPS_005.00 @ 0x80011CFC, 0x80011DD0
 *
 * Callers fill in g_cd_queue and submit a count; CdQueueDispatch then starts
 * request g_cd_queue_index, either as a plain read or, when the mode byte has
 * bit 7 set, as a streaming read.
 */
#include <libcd.h>
#include <persona/main/cd.h>

/* Only ever written, by CdQueueDispatch. Nothing on the disc reads either one -
   not the overlays, not any of the four sub-EXEs - so they are a record of the
   streaming request in flight and nothing more; volatile is what keeps the
   stores in a build that never reads them back. Nothing outside this source
   touches them, so they are not in persona/main/cd.h. */
extern volatile u_short g_cd_stream_mode;    /* mode, streaming flag stripped */
extern volatile int     g_cd_stream_sectors; /* size rounded up to sectors    */

/* CdQueueDispatch and CdQueueNextCallback call each other: one request's
   completion starts the next. */
void CdQueueDispatch(void);

/* Clears whichever callback the current request installed - the read-done one
   for a plain request, the data-ready one for a streaming request. `unused`
   reserves stack the original reserves; deleting it breaks the match. */
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
   The duplicated `return -3` has to stay; collapsing the two breaks the
   match. */
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
   lets the per-sector ready interrupt drive it, after recording the mode with
   the streaming flag stripped and the size rounded up to sectors. */
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
   `unused` reserves the same stack frame CdQueueSubmit has; deleting it breaks
   the match. */
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
