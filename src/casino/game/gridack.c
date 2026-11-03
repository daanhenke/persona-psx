#include <persona/casino/casino.h>
#include <libsnd.h>

extern signed char D_800AF73C[];
extern CasinoLayout *D_800A2BC8[];
extern void CasinoSpritesSetOn(int, int, int);
extern void CasinoTween(CasinoLayout *, int, short, short, short, short);

void func_80089E7C(void)
{
    if (!(g_casino_timer & 31))
        CasinoSpritesSetOn(16, 1, (g_casino_timer / 32) % 2);
    if (g_casino_timer >= 32)
    {
        if (g_casino_pad_trig & 8192 || g_casino_pad_trig & 32768 ||
            g_casino_pad_trig & 4096 || g_casino_pad_trig & 16384 ||
            g_casino_pad_trig & 32 || g_casino_pad_trig & 128 ||
            g_casino_pad_trig & 16 || g_casino_pad_trig & 64 ||
            g_casino_pad_trig & 4 || g_casino_pad_trig & 1 ||
            g_casino_pad_trig & 8 || g_casino_pad_trig & 2 ||
            g_casino_pad_trig & 256)
            D_800AF73C[2]++;
        if (D_800AF73C[2])
        {
            CasinoTween(D_800A2BC8[217], -128, 0, 0, 0, 32);
            SsSeqStop(g_casino_seqs[4]);
            SsSeqPlay(g_casino_seqs[4], 1, 1);
            D_800AF73C[2] = 0;
            CasinoSpritesSetOn(16, 1, 1);
            g_casino_step = 120;
            g_casino_timer = -1;
        }
    }
}
