/* Persona 1 (JP) - silencing the music on the way out of ADV.
 *
 *   ADV @ 0x8007E5A4
 *
 * ovl_adv_entry calls this just before AdvCloseBanks. Where the player is
 * going decides whether the sequencer is stopped: modes 1, 5 and 6 always
 * silence it, 0 and 2 only when the flag is set, and anything else leaves the
 * music playing into whatever loads next.
 */
#include <types.h>

extern short g_adv_enter_mode;
extern short g_dng_third_gate;
extern short g_bgm_seq;

extern void SsSetNck(short seq);
extern void SsSetMVol(short left, short right);

void AdvSilenceBgm(void)
{
    switch (g_adv_enter_mode) {
    case 0:
    case 2:
        if (g_dng_third_gate == 0) {
            return;
        }
    case 1:
    case 5:
    case 6:
        SsSetNck(g_bgm_seq);
        SsSetMVol(0, 0);
    }
}
