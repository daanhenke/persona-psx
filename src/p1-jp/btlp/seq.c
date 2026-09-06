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

extern int  BtlSeqState(void);
extern void BtlSeqSetState(int a, int b);
extern void BtlIndicatorClear(void);
extern void BtlRunFrames(int arg);
extern void BtlUpdateVoices(void);
extern void BtlDrawFrame(void);
extern void BtlSeqWaitDone(void);

void BtlWaitBgmEnd(void)
{
    if (g_btl_bgm_state == 1 && g_btl_bgm_seq < 0) {
        while (BtlSeqState() != 0) {
            if (SsIsEos(g_btl_seq_handle, (short)g_btl_bgm_seq) == 0) {
                BtlSeqSetState(0, 4);
                BtlIndicatorClear();
                BtlRunFrames(0x1E);
            }
            BtlUpdateVoices();
            BtlDrawFrame();
        }
    } else {
        BtlSeqWaitDone();
    }
}
