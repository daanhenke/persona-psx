#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <persona/casino/casino.h>
#include <persona/casino/blackjack.h>

extern CasinoObj D_8009C558;
extern void CasinoQueueCluts();
extern void CasinoStartAnim();
extern void CasinoPlaySeq(short*, u_int, short);

void func_8007A77C(void)
{
    if (g_casino_timer == 0)
    {
        CasinoQueueCluts(0x38F, 12, 0x7CE4);
        CasinoStartAnim(&D_8009C558, 32, 0, 2048, 0, 0, 0, 0, 0, 0, 0);
    }
    if (g_casino_timer == 28)
    {
        CasinoPlaySeq(g_casino_seqs, 0x138B64, g_casino_main_vab);
    }
    if (g_casino_timer >= 61)
    {
        g_casino_bet--;
        g_casino_money++;
        if (g_casino_bet == 0)
        {
            CasinoStartAnim(&D_8009C558, 32, 0, 0, 0, -320, 0, 0, 0, 0, 0);
            g_bj_play_step = 0x30;
            g_casino_timer = -1;
        }
    }
}
