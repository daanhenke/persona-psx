/* Fade the second blackjack hand and its labels. */
#include <persona/casino/blackjack.h>

extern u_char D_800A8678[];

void func_8007F668(u_char mode)
{
    u_char level = 0;
    D_800A8678[1] = mode;
    if (mode == 1)
        level = 128;
    else if (mode == 2)
        level = 64;

    CasinoFade(580, 20, level, level, level, 8);
    CasinoFade(520, 20, level, level, level, 8);
    if (g_bj_count[1] >= 3)
        CasinoFade(620 - (g_bj_count[1] + g_bj_count[0] + 1) * 20,
                   (g_bj_count[1] - 2) * 20, level, level, level, 8);
    CasinoFade(694, 3, level, level, level, 8);
    CasinoFade(727, 10, level, level, level, 8);
    if (g_bj_natural[1] != 1 && g_bj_natural[1] != 2)
        CasinoFade(891, 8, level, level, level, 8);
    else if (g_bj_natural[1] == 2)
        CasinoFade(907, 4, level, level, level, 8);
}
