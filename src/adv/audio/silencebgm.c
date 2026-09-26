/* Persona 1 (JP) - silencing the music on the way out of ADV.
 *
 *   ADV 0x8007E5A4 AdvSilenceBgm   0x8007E618 AdvFadeBgmOut
 *       0x8007E650 AdvLeaveMapFlag
 *
 * ovl_adv_entry calls this just before AdvCloseBanks. Where the player is
 * going decides whether the sequencer is stopped: modes 1, 5 and 6 always
 * silence it, 0 and 2 only when music was started, and anything else leaves
 * the music playing into whatever loads next.
 */
#include <decomp/types.h>

extern short g_adv_enter_mode;
extern short g_bgm_ready;
extern u_short g_prev_map;

#define g_seq_handle ((short *)0x801F537C)

extern void SsSetNck(short seq);
extern void SsSetMVol(short left, short right);
extern void SsSeqSetDecrescendo(short seq, short vol, short time);
extern void EventFlagSet(u_short id);

void AdvSilenceBgm(void)
{
    switch (g_adv_enter_mode) {
    case 0:
    case 2:
        if (g_bgm_ready == 0) {
            return;
        }
    case 1:
    case 5:
    case 6:
        SsSetNck(g_seq_handle[0]);
        SsSetMVol(0, 0);
    }
}

/* Fades the music down over 0x20 ticks as the scene is left, if any is
   playing. */
void AdvFadeBgmOut(void)
{
    if (g_bgm_ready) {
        SsSeqSetDecrescendo(g_seq_handle[0], 0x7F, 0x20);
    }
}

/* Leaving for another overlay from one of these maps sets event flag 0x12C. */
void AdvLeaveMapFlag(void)
{
    switch (g_prev_map) {
    case 0x10:
    case 0x230:
    case 0x26B:
    case 0x26C:
    case 0x2B9:
    case 0x2C3:
    case 0x2CE:
    case 0x309:
        EventFlagSet(0x12C);
    }
}
