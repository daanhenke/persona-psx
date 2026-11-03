#include <persona/casino/blackjack.h>

extern u_char D_800A8768[2];
extern u_char D_800A8678[2];
extern u_char D_800B09F4[2];
extern void CasinoTween();

void func_8007AFC0(void)
{
    if (g_casino_timer == 0)
    {
        D_800A8768[0] = 0xED;
        D_800A8768[1] = 0xED;
        g_bj_count[0] = 1;
        g_bj_count[1] = 1;
        g_bj_total[0] = 0;
        g_bj_total[1] = 0;
        g_bj_split_total[0] = 0;
        g_bj_split_total[1] = 0;
        g_bj_natural[0] = 1;
        g_bj_natural[1] = 1;
        D_800A8678[0] = 1;
        D_800A8678[1] = 1;
        g_bj_cards[1][0] = g_bj_cards[0][1];
        g_casino_jackpot_add = g_casino_bet;
        D_800B09F4[1] = g_casino_bet;
        D_800B09F4[0] = g_casino_bet;
        g_casino_money -= g_casino_bet;
    }
    if (g_casino_timer >= 8)
    {
        CasinoTween(D_8009AD88[0].l, 0, 8, 0, -16, 8);
        CasinoTween(D_8009AD88[1].l, 0, 8, 0, -16, 8);
        g_casino_timer = -1;
        g_bj_label_n[1] = 0;
        g_bj_label_n[2] = 0;
        g_bj_play_step = 0x2D;
    }
}
