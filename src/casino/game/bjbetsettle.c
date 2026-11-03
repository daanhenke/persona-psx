#include <persona/casino/blackjack.h>

extern CasinoObj D_8009C42C[], D_8009C568[];
extern u_char D_800A8678[], D_800B09F4[];
extern void CasinoStartAnim(CasinoObj *, short, short, short, short,
                            long, long, long, long, long, long);
extern void CasinoPlaySeq(short *, u_long *, short);
extern void func_8007F7E8(u_char), func_8007F668(u_char);

void func_8007C8E4(u_char hand, u_char next)
{
    if (g_casino_timer == 0)
    {
        if (g_bj_natural[hand] != 2)
        {
            CasinoStartAnim(&D_8009C42C[hand], 32, 0, 2048, 0, 0, 0, 0, 0, 0, 0);
            g_bj_natural[hand] = 12;
        }
        if (D_800A8678[hand] == 2)
        {
            if (hand == 0)
                func_8007F7E8(1);
            else
                func_8007F668(1);
        }
    }
    if (g_casino_timer == 16)
        CasinoPlaySeq(g_casino_seqs, (u_long *)0x138B0C, g_casino_main_vab);
    if (g_casino_timer >= 61)
    {
        D_800B09F4[hand]--;
        if (D_800B09F4[hand] == 0)
        {
            g_casino_timer = -1;
            g_bj_play_step = next;
        }
    }
}

void func_8007CA5C(u_char hand, u_char next)
{
    if (g_casino_timer == 0)
    {
        if (g_bj_natural[hand] == 1)
        {
            CasinoStartAnim(&D_8009C568[hand], 32, 0, 2048, 0, 0, 0, 0, 0, 0, 0);
            g_bj_natural[hand] = 13;
            if (D_800A8678[hand] == 2)
            {
                if (hand == 0)
                    func_8007F7E8(1);
                else
                    func_8007F668(1);
            }
        }
    }
    if (g_casino_timer == 16)
        CasinoPlaySeq(g_casino_seqs, (u_long *)0x138B64, g_casino_main_vab);
    if (g_casino_timer >= 61)
    {
        D_800B09F4[hand]--;
        g_casino_money++;
        if (D_800B09F4[hand] == 0)
        {
            g_casino_timer = -1;
            g_bj_play_step = next;
        }
    }
}
