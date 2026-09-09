/* Persona 1 (JP) - running the CD request queue.
 *   SLPS_005.00 @ 0x80012754 CdQueueNextCallback, 0x800127F0 CdQueueReadyCallback
 *                 0x80012830 CdQueueDispatch,    0x80012A10 CdQueueClearCallback
 *
 * The four sit in the image in that order, which is not the order they read
 * in: the read-done callback comes first and the callback-clearing helper it
 * calls last, so the forward declaration below is what the original call site
 * was compiled against. Submitting a queue is a unit of its own well ahead of
 * this, in cdqueue.c.
 */
#include <decomp/types.h>
#include <libcd.h>
#include <persona/main/cd.h>

/* Only ever written, by CdQueueDispatch. Nothing on the disc reads either one -
   not the overlays, not any of the four sub-EXEs - so they are a record of the
   streaming request in flight and nothing more; volatile is what keeps the
   stores in a build that never reads them back. */
extern volatile u_short g_cd_stream_mode;    /* mode, streaming flag stripped */
extern volatile int     g_cd_stream_sectors; /* size rounded up to sectors    */

/* Defined at the end of this file, but called from the top of it. */
void CdQueueClearCallback(void);

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

/* Data-ready callback for the streaming path. Only a disk error is acted on;
   a normal completion reports whatever state the drive is already in. The two
   error test is written the way round it is - bail out when the status is not a
   disk error - so that the branch lands on the epilogue with -3 already in the
   return register from its delay slot, and the store below reads it back from
   there. Testing for the error instead costs a second copy of the constant
   however the returns are arranged. */
int CdQueueReadyCallback(u_char status)
{
    CdlLOC unused[3];

    if (status == CdlComplete) {
        return g_cd_busy;
    }
    if (status != CdlDiskError) {
        return -3;
    }
    g_cd_busy = -3;
    return -3;
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
