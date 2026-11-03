extern unsigned char g_bj_natural[], g_bj_dealer_bj;
extern unsigned char g_bj_total[], g_bj_split_total[], g_bj_dealer_total[];
extern unsigned char D_800A8768[], g_bj_play_step;
extern int g_casino_timer;
extern int func_80082400(unsigned char, unsigned char, unsigned char, unsigned char);

void func_8007C158(void)
{
    switch (g_bj_natural[0])
    {
    case 1:
        D_800A8768[0] = func_80082400(g_bj_natural[0], g_bj_dealer_bj,
                                    g_bj_total[1], g_bj_dealer_total[1]);
        if (g_bj_dealer_bj == 8)
            D_800A8768[0] = 12;
        break;
    case 2:
        D_800A8768[0] = 12;
        break;
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 9:
    case 10:
        D_800A8768[0] = 11;
        break;
    }
    switch (g_bj_natural[1])
    {
    case 1:
        D_800A8768[1] = func_80082400(g_bj_natural[1], g_bj_dealer_bj,
                                    g_bj_split_total[1], g_bj_dealer_total[1]);
        if (g_bj_dealer_bj == 8)
            D_800A8768[1] = 12;
        break;
    case 2:
        D_800A8768[1] = 12;
        break;
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 9:
    case 10:
        D_800A8768[1] = 11;
        break;
    }
    g_casino_timer = -1;
    g_bj_play_step = 120;
}
