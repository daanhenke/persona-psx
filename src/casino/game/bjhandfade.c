#include <persona/casino/blackjack.h>

extern u_char D_800A8678[];

void func_8007F7E8(u_char mode)
{
    u_char level = 0;
    D_800A8678[0] = mode;
    if (mode == 1)
        level = 128;
    else if (mode == 2)
        level = 64;

    CasinoFade(620, 20, level, level, level, 8);
    CasinoFade(540, 20, level, level, level, 8);
    if (g_bj_count[0] >= 3)
        CasinoFade(620 - (g_bj_count[0] + 3) * 20,
                   (g_bj_count[0] - 2) * 20, level, level, level, 8);
    CasinoFade(691, 3, level, level, level, 8);
    CasinoFade(717, 10, level, level, level, 8);
    if (g_bj_natural[0] != 1 && g_bj_natural[0] != 2)
        CasinoFade(883, 8, level, level, level, 8);
    else if (g_bj_natural[0] == 2)
        CasinoFade(903, 4, level, level, level, 8);
}
