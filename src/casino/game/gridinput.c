/* Grid-game input, retry sequence, and prompt/lamp sequencing. */
#include <persona/casino/casino.h>

extern u_char D_800A878C;
extern s8 D_800AF738, D_800AF73C[], D_800A966C, D_800A9670;
/* These independent counters share the overlay's work area. */
extern short grid_tick __asm__("D_800AF9E8+0xD0");
extern short lamp_tick __asm__("D_800AF9E8+0xD4");
extern void func_8008A9DC(int, int, int);
extern void SsSeqStop(short), SsSeqPlay(short, char, short);
extern void func_8008C4D0(void), func_8008C16C(void);

void func_80088A28(void)
{
    if (!g_casino_timer) {
        if (D_800A878C)
            CASINO_LAMPS.b2 = 1;
        else
            CASINO_LAMPS.b2 = 0;
        grid_tick = 0;
        D_800A966C = 1;
        lamp_tick = 0;
        D_800A9670 = 0;
        CASINO_LAMPS.b0 = 1;
        func_8008A9DC(0, 0, 1);
    }
    if (g_casino_timer > 16) {
        if ((g_casino_pad_trig & 4 || g_casino_pad_trig & 1) &&
            !D_800AF738 && !g_casino_bet_done)
            g_casino_quit = 1;
        else if ((g_casino_pad_trig & 8 || g_casino_pad_trig & 2) &&
                 !D_800AF73C[2] && !g_casino_quit)
            D_800AF73C[2]++;
    }
    if (D_800AF73C[2]) {
        if (D_800A878C) {
            SsSeqStop(g_casino_seqs[4]);
            SsSeqPlay(g_casino_seqs[4], 1, 1);
            g_casino_bet_done = 1;
            D_800AF73C[2] = 0;
            D_800A878C--;
        } else if (D_800AF73C[2]) {
            D_800A9670 = 1;
            D_800A966C = 0;
            lamp_tick = 0;
            D_800AF73C[2] = 0;
        }
    }
    if (D_800A9670 == 1)
        func_8008C4D0();
    if (D_800A966C == 1)
        func_8008C16C();
}
