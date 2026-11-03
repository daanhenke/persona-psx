#include <persona/casino/blackjack.h>

extern CasinoObj D_8009C41C;
extern void CasinoQueueCluts();
extern void CasinoStartAnim();
extern void CasinoPlaySeq(short *, u_int, short);

void func_8007A654(void)
{
    if (g_casino_timer == 0 && g_bj_natural[0] != 2)
    {
        CasinoQueueCluts(0x383, 12, 0x7CA4);
        CasinoStartAnim(&D_8009C41C, 32, 0, 2048, 0, 0, 0, 0, 0, 0, 0);
    }
    if (g_casino_timer == 28 && g_bj_natural[0] != 2)
    {
        CasinoPlaySeq(g_casino_seqs, 0x138B0C, g_casino_main_vab);
    }
    if (g_casino_timer >= 73)
    {
        CasinoStartAnim(&D_8009C41C, 32, 0, 0, 0, -320, 0, 0, 0, 0, 0);
        g_bj_play_step = 0x30;
        g_casino_bet = 0;
        g_casino_timer = -1;
    }
}
