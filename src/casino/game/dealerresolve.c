#include <persona/casino/blackjack.h>

extern u_char D_800A8C7C;
extern u_char func_8008230C(void *), func_80081DC0(void *);
extern void CasinoPlaySeq(short *, unsigned int, short);
extern void func_8007EB20(u_char, u_char, void *, void *, void *, int, int);

void func_8007D554(void)
{
    u_char flag = 0;

    if (func_8008230C(g_bj_dealer_total) == 2)
    {
        g_bj_dealer_bj = 2;
        D_800A8C7C = 113;
    }
    if (func_80081DC0(g_bj_dealer) != 1)
    {
        flag = 1;
        if (g_bj_natural[0] == 1 || g_bj_natural[0] == 2)
            CasinoPlaySeq(&g_casino_seqs[2], 0x1384C4, g_casino_vab);
        g_bj_dealer_bj = 8;
        D_800A8C7C = 113;
    }
    if (g_bj_dealer_total[0] < 17)
        D_800A8C7C = 112;
    if (g_bj_dealer_total[1] >= 17)
        D_800A8C7C = 113;
    if (g_bj_dealer_total[0] == 7 && g_bj_dealer_total[1] == 17)
        D_800A8C7C = 112;
    if (flag)
        func_8007EB20(g_bj_dealer_bj, g_bj_dealer_total[1], D_8009B2B4[0].l,
                     g_bj_label_n, D_8009ACE4, 0, 8);
    else
        func_8007EB20(g_bj_dealer_bj, g_bj_dealer_total[1], D_8009B2B4[0].l,
                     g_bj_label_n, D_8009ACE4, 1, 8);
    g_casino_timer = -1;
}
