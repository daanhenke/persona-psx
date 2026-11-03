extern unsigned int g_casino_money, g_casino_money_shown, g_bj_jackpot, g_casino_jackpot_shown;
extern int D_800A8EA4, g_casino_win, g_casino_frame, g_casino_timer;
extern unsigned char g_casino_jackpot_add, g_casino_bet, g_casino_bet_shown, D_800A8674,
    D_800A8818[];
extern char g_bj_money_digits[], g_bj_win_digits[], g_bj_jackpot_digits[], g_bj_bet_digits[],
    D_8009C078[], D_8009C108[], D_8009B7B0[];
extern unsigned char g_bj_double_palette_on;
extern char D_800999A8[];
extern struct
{
    unsigned a : 1;
    unsigned b : 1;
    unsigned c : 1;
    unsigned d : 1;
    unsigned e : 1;
    unsigned f : 1;
} g_casino_lamps;
extern void CasinoShowNumber(int, void *, int), CasinoShowCents(int, void *, int),
    CasinoStartPalAnim(void *),
    CasinoStartAnim(void *, int, int, int, int, int, int, int, int, int, int);
void func_80080F1C(void)
{
    if (g_casino_money != g_casino_money_shown)
    {
        CasinoShowNumber(g_casino_money, g_bj_money_digits, 8);
        g_casino_money_shown = g_casino_money;
    }
    if (D_800A8EA4 != g_casino_win)
    {
        CasinoShowNumber(g_casino_win, g_bj_win_digits, 4);
        D_800A8EA4 = g_casino_win;
    }
    if (g_casino_jackpot_add && !(g_casino_frame & 1))
    {
        if (g_bj_jackpot < 999999)
            g_bj_jackpot++;
        g_casino_jackpot_add--;
    }
    if (g_bj_jackpot != g_casino_jackpot_shown)
    {
        g_casino_jackpot_shown = g_bj_jackpot;
        CasinoShowCents(g_bj_jackpot, g_bj_jackpot_digits, 6);
    }
    if (g_casino_bet != g_casino_bet_shown && g_casino_timer)
    {
        CasinoShowNumber(g_casino_bet, g_bj_bet_digits, 2);
        g_casino_bet_shown = g_casino_bet;
    }
    if (D_800A8818[D_800A8674] == 1 && !g_bj_double_palette_on)
        CasinoStartPalAnim(&D_800999A8);
    if (g_casino_lamps.a != g_casino_lamps.b)
    {
        CasinoStartAnim(D_8009C078, 16, 2048, 0, 0, 0, 0, 0, 0, 0, 0);
        g_casino_lamps.b = g_casino_lamps.a;
    }
    if (g_casino_lamps.c != g_casino_lamps.d)
    {
        CasinoStartAnim(D_8009C108, 16, 2048, 0, 0, 0, 0, 0, 0, 0, 0);
        g_casino_lamps.d = g_casino_lamps.c;
    }
    if (g_casino_lamps.e != g_casino_lamps.f)
    {
        CasinoStartAnim(D_8009B7B0, 32, g_casino_lamps.e ? 2048 : -2048, 0, 0, 0, 0, 0, 0, 0, 0);
        g_casino_lamps.f = g_casino_lamps.e;
    }
}
