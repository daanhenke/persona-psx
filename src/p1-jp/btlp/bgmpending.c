/* Persona 1 (JP) - BTLP overlay @ 0x8007EF38
 *
 * g_btl_bgm_pending holds the track a caller has asked for but which has not
 * started yet; clearing it cancels the request. Every path that stops or
 * replaces the music clears it here first, so a queued track cannot start on
 * top of whatever comes next.
 */
#include <types.h>

extern short g_btl_bgm_pending;

void BtlBgmClearPending(void)
{
    g_btl_bgm_pending = 0;
}
