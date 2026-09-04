/* Persona 1 (JP) - BTLP overlay @ 0x80066E50
 *
 * Drives a sequence until it ends: while the pump returns non-zero, poll
 * SsIsEos and, on end-of-sequence, kick the three follow-up handlers.
 * If the pair of guards up front fails, take the single fallback call instead.
 */
#include <types.h>
#include <libsnd.h>

extern int   g_btl_bgm_state;
extern int   g_btl_bgm_seq;
extern short g_btl_seq_handle;

extern int  BtlSeqIsActive(void);
extern void BtlSeqSetState(int a, int b);
extern void BtlBgmClearPending(void);
extern void BtlRunFrames(int arg);
extern void BtlUpdateVoices(void);
extern void BtlDrawFrame(void);
extern void BtlSeqWaitDone(void);

void BtlWaitBgmEnd(void)
{
    if (g_btl_bgm_state == 1 && g_btl_bgm_seq < 0) {
        while (BtlSeqIsActive() != 0) {
            if (SsIsEos(g_btl_seq_handle, (short)g_btl_bgm_seq) == 0) {
                BtlSeqSetState(0, 4);
                BtlBgmClearPending();
                BtlRunFrames(0x1E);
            }
            BtlUpdateVoices();
            BtlDrawFrame();
        }
    } else {
        BtlSeqWaitDone();
    }
}
