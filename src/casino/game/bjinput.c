/* Persona 1 (JP) - process blackjack confirm/advance input. */
#include <decomp/types.h>
extern u_long g_casino_pad_trig;
extern u8 g_bj_play_step;
extern u8 g_bj_natural;
extern u8 g_bj_total[];
extern u8 g_bj_label_n[];
extern u8 natural_label_pending __asm__("D_800AAEA8+0x4");
extern int D_8009B2B4[];
extern char D_8009AD88[];
extern int g_casino_timer;
extern short g_casino_seqs[];
extern void SsSeqStop(short);
extern void SsSeqPlay(short, short, short);
extern void func_8007F368(int, int);
extern void func_8007EB20(int, u8, int, u8 *, char *, int, int);
void func_800815D8(void) {
    if (g_casino_pad_trig & 0x40) {
        g_bj_play_step = 0x71;
    } else if (g_casino_pad_trig & 0x20) {
        g_bj_play_step = 0x70;
    }
    if (g_bj_play_step != 0x1B) {
        SsSeqStop(g_casino_seqs[4]);
        SsSeqPlay(g_casino_seqs[4], 1, 1);
        g_casino_timer = -1;
        func_8007F368(0, 0);
        func_8007F368(0, 2);
        if ((g_bj_natural == 0xA) && (natural_label_pending == 0)) {
            func_8007EB20(0xA, g_bj_total[1], D_8009B2B4[3], g_bj_label_n + 1, D_8009AD88, 0, 1);
            natural_label_pending = 0U;
        }
    }
}
