/* Persona JP1 dice result and reward state. */
extern int g_casino_timer;
extern unsigned char g_casino_step, D_800AAE9C[], D_800A9724[], D_800A96FC[10][2], D_800A970E,
    D_800A970F;
extern short D_800B0720[];
extern void CasinoSpritesSetOn(int, int, int), func_800911E4(short *, unsigned char *),
    func_80093614(void), func_800913FC(unsigned char), func_80093950(unsigned char);
extern unsigned char func_800915E4(unsigned char, void *);
void func_8008D6EC(void)
{
    int i;
    if (!g_casino_timer)
    {
        (*(unsigned int *)(D_800AAE9C + 4))++;
        CasinoSpritesSetOn(298, 24, 1);
        CasinoSpritesSetOn(322, 7, 1);
        CasinoSpritesSetOn(329, 6, 1);
        CasinoSpritesSetOn(335, 12, 1);
        CasinoSpritesSetOn(347, 8, 1);
        CasinoSpritesSetOn(355, 6, 1);
        CasinoSpritesSetOn(361, 8, 1);
        func_800911E4(D_800B0720, D_800A9724);
        func_800911E4(D_800B0720 + 120, D_800A9724 + 1);
        D_800A9724[2] = D_800A9724[0] + D_800A9724[1];
    }
    if (g_casino_timer == 8)
    {
        if (*(unsigned int *)(D_800AAE9C + 4) <= 10)
        {
            CasinoSpritesSetOn(*(unsigned int *)(D_800AAE9C + 4) * 4 + 366, 4, 1);
            D_800A96FC[*(unsigned int *)(D_800AAE9C + 4) - 1][0] = D_800A9724[0];
            D_800A96FC[*(unsigned int *)(D_800AAE9C + 4) - 1][1] = D_800A9724[1];
        }
        else
        {
            for (i = 0; i < 9; i++)
            {
                D_800A96FC[i][0] = D_800A96FC[i + 1][0];
                D_800A96FC[i][1] = D_800A96FC[i + 1][1];
            }
            D_800A970E = D_800A9724[0];
            D_800A970F = D_800A9724[1];
        }
        func_80093614();
        func_800913FC(D_800A9724[2]);
    }
    if (g_casino_timer == 24)
    {
        func_80093950(D_800A9724[2]);
        if (func_800915E4(D_800A9724[2] - 2, D_800A9724 - 128))
        {
            if (D_800AAE9C[2] == 0)
                g_casino_step = 114;
            else if (D_800AAE9C[2] == 1)
                g_casino_step = 114;
        }
        else
            g_casino_step = 115;
        g_casino_timer = -1;
    }
}
