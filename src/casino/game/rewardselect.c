/* Select whether to continue a dice reward round, restoring the result display. */
#include <persona/casino/reward.h>
#include <libsnd.h>
extern u_char D_800AAE9C[], g_casino_wrapped;
extern CasinoLayout *D_800A2BC8[];
extern void func_800915FC(void), func_80091544(int);
extern void CasinoPlaySeq(short *, u_long *, short);
extern void CasinoTween(CasinoLayout *, int, short, short, short, short);
extern void CasinoCloseSeq(short *);
extern void CasinoQueueCluts(short, short, int);
extern void CasinoSpritesSetOn(short, short, int);

static inline void Digits(short n)
{
    if (g_casino_reward_data.values[(int)((u_int)n << 16) >> 16] < 10)
        CasinoSpritesSetOn(n * 4 + 67, 3, 0);
    else if (g_casino_reward_data.values[(int)((u_int)n << 16) >> 16] < 100)
        CasinoSpritesSetOn(n * 4 + 67, 2, 0);
    else if (g_casino_reward_data.values[(int)((u_int)n << 16) >> 16] < 1000)
        CasinoSpritesSetOn(n * 4 + 67, 1, 0);
}

void func_8008E0C0(void)
{
    u_char selected = 0;
    if (g_casino_timer > 0) func_800915FC();
    if (g_casino_timer == 0) {
        CasinoPlaySeq(g_casino_seqs, (u_long *)0x13AA88, g_casino_vab);
        CasinoTween(D_800A2BC8[0x1C70 / 4], 0, -80, 0, 0, 16);
        if (D_800AAE9C[2] == 0) {
            CasinoTween(D_800A2BC8[0x2DB8 / 4], 0, -208, 0, 0, 16);
            CasinoTween(D_800A2BC8[0x2DC0 / 4], 0, -208, 0, 0, 16);
        } else if (D_800AAE9C[2] == 1)
            CasinoTween(D_800A2BC8[0x2EF0 / 4], 0, -208, 0, 0, 16);
        CasinoTween(D_800A2BC8[2947], 0, -208, 0, 0, 16);
        CasinoTween(D_800A2BC8[2965], 0, -208, 0, 0, 16);
        CasinoTween(D_800A2BC8[2986], 0, -208, 0, 0, 16);
        func_80091544(g_casino_reward_data.dice[2]);
    }
    if (g_casino_timer == 16) {
        if (D_800AAE9C[2] == 1) CasinoPlaySeq(g_casino_seqs + 1, (u_long *)0x13B0BC, g_casino_main_vab);
    }
    if (g_casino_timer == 62 && D_800AAE9C[2] == 1)
        CasinoPlaySeq(g_casino_seqs, (u_long *)0x13AB54, g_casino_vab);
    if (g_casino_timer > 16) {
        if ((g_casino_pad_trig & 32) && !(g_casino_pad_trig & 64)) {
            D_800AAE9C[9] = 1;
            selected = 1;
            g_casino_step = 51;
            g_casino_timer = -1;
        } else if ((g_casino_pad_trig & 64) && !(g_casino_pad_trig & 32)) {
            g_casino_step = 49;
            g_casino_timer = -1;
        }
    }
    if (g_casino_step != 41) {
        SsSeqStop(g_casino_seqs[4]);
        SsSeqPlay(g_casino_seqs[4], 1, 1);
        CasinoTween(D_800A2BC8[0x1C70 / 4], 0, 240, 0, 0, 16);
        if (D_800AAE9C[2] == 0) {
            CasinoTween(D_800A2BC8[0x2DB8 / 4], 0, 208, 0, 0, 16);
            CasinoTween(D_800A2BC8[0x2DC0 / 4], 0, 208, 0, 0, 16);
        } else if (D_800AAE9C[2] == 1) {
            SsSeqSetVol(g_casino_seqs[15], 127, 127);
            if (g_casino_wrapped) SsSeqStop(g_casino_seqs[15]);
            SsSeqPlay(g_casino_seqs[15], 1, 1);
            CasinoCloseSeq(g_casino_seqs);
            CasinoTween(D_800A2BC8[0x2EF0 / 4], 0, 208, 0, 0, 16);
        }
        CasinoTween(D_800A2BC8[2947], 0, 208, 0, 0, 16);
        CasinoTween(D_800A2BC8[2965], 0, 208, 0, 0, 16);
        CasinoTween(D_800A2BC8[2986], 0, 208, 0, 0, 16);
        CasinoQueueCluts(g_casino_reward_data.dice[2] + 62, 1, 0x7CA5);
        CasinoSpritesSetOn(g_casino_reward_data.dice[2] * 4 + 67, 4, 1);
        CasinoSpritesSetOn(g_casino_reward_data.dice[2] * 2 + 115, 2, 1);
        CasinoSpritesSetOn(g_casino_reward_data.dice[2] * 2 + 137, 2, 1);
        Digits(g_casino_reward_data.dice[2]);
        D_800AAE9C[2] = selected;
    }
}
