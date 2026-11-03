#include <persona/casino/blackjack.h>

extern u_char D_800A8C7C;
extern CasinoObj D_8009C41C;
extern void CasinoQueueCluts(int, int, u_short);
extern void CasinoStartAnim(CasinoObj *, short, short, short, short,
                            long, long, long, long, long, long);
extern void CasinoPlaySeq(short *, u_long *, short);

void func_8007A10C(void)
{
    if (g_casino_timer == 0)
    {
        switch (g_bj_natural[0])
        {
        case 1:
        case 8:
            g_bj_play_step = 116;
            D_800A8C7C = 18;
            g_casino_timer = -1;
            break;
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
        case 9:
        case 10:
            g_bj_play_step = 22;
            g_casino_timer = -1;
            break;
        }
    }
    if (g_bj_natural[0] == 2 && g_casino_timer == 8)
    {
        CasinoQueueCluts(899, 12, 0x7CA4);
        CasinoStartAnim(&D_8009C41C, 32, 0, 2048, 0, 0, 0, 0, 0, 0, 0);
    }
    if (g_bj_natural[0] == 2 && g_casino_timer >= 33)
    {
        CasinoPlaySeq(g_casino_seqs, (u_long *)0x138B0C, g_casino_main_vab);
        g_bj_play_step = 116;
        D_800A8C7C = 18;
        g_casino_timer = -1;
    }
}
