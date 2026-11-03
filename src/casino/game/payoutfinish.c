extern int g_casino_timer, g_casino_win, g_casino_frame, D_800A96A4[];
extern unsigned int g_casino_money;
extern unsigned char g_casino_step;
extern void func_80090EEC(unsigned char);
extern void CasinoSplitDigits(int);
extern void CasinoPayStep(int *);

void func_8008E6F0(void)
{
    if ((unsigned int)g_casino_timer < 11) {
        D_800A96A4[g_casino_timer] = 0;
        func_80090EEC(g_casino_timer);
    }
    if (!g_casino_timer)
        CasinoSplitDigits(g_casino_win);
    if (g_casino_win) {
        if (!(g_casino_frame & 1))
            CasinoPayStep(&g_casino_win);
        if (g_casino_money >= 99999999U) {
            g_casino_win = 0;
            g_casino_money = 99999999;
        }
    }
    if (!g_casino_win && g_casino_timer >= 11) {
        g_casino_step = 36;
        g_casino_timer = -1;
    }
}
