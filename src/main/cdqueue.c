/* Persona 1 (JP) - CD request queue.
 *   SLPS_005.00 @ 0x80011CFC CdQueueSubmit, 0x80011DD0 CdQueueSubmitResolved
 *
 * Callers fill in g_cd_queue and submit a count; CdQueueDispatch then starts
 * request g_cd_queue_index, either as a plain read or, when the mode byte has
 * bit 7 set, as a streaming read. The dispatcher and the three callbacks
 * around it are a unit of their own further along, in cdqueuerun.c.
 */
#include <decomp/types.h>
#include <libcd.h>
#include <persona/main/cd.h>

/* In cdqueuerun.c, a unit of its own. */
void CdQueueDispatch(void);

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
