#include <persona/casino/casino.h>
#include <libsnd.h>

extern CasinoPalAnim D_80099CA8;
extern unsigned char g_casino_reward_palette_on;
extern unsigned char D_800AAE9C[];
extern void CasinoStartPalAnim(CasinoPalAnim *);
extern void func_80093AD0(unsigned int);

void func_8008DC54(void)
{
    unsigned char done = 0;

    if (!g_casino_reward_palette_on)
        CasinoStartPalAnim(&D_80099CA8);
    if (!g_casino_timer) {
        if (D_800AAE9C[2] == 1)
            SsSeqSetVol(g_casino_seqs[15], 0, 0);
        func_80093AD0(g_casino_win);
    }
    if (!SsIsEos(g_casino_seqs[1], 0))
        done = 1;
    if (g_casino_timer >= 0 && done) {
        if (g_casino_win == 10000)
            g_casino_step = 117;
        else
            g_casino_step = 41;
        g_casino_timer = -1;
    }
}
