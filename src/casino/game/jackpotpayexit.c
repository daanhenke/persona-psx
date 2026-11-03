#include <persona/casino/blackjack.h>

extern CasinoObj D_8009C274[2];
extern void CasinoPayStep(int *);
extern void CasinoStartAnim();

void func_8007A8B4(void)
{
    if (!(g_casino_frame & 1))
    {
        CasinoPayStep(&g_casino_win);
    }
    if (g_casino_money > 99999998U)
    {
        g_casino_win = 0;
        g_casino_money = 99999999;
    }
    if (g_casino_win == 0)
    {
        if (g_bj_natural[0] == 0 && g_casino_bet >= 10)
        {
            g_bj_jackpot = 300000;
        }
        CasinoStartAnim(&D_8009C274[0], 32, 0, 0, 0, -320, 0, 0, 0, 0, 0);
        CasinoStartAnim(&D_8009C274[1], 32, 0, 0, 0, -320, 0, 0, 0, 0, 0);
        g_casino_timer = -1;
        g_bj_play_step = 0x30;
    }
}
