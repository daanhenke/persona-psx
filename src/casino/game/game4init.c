#include <persona/casino/casino.h>

extern int g_casino_timer;
extern unsigned char g_casino_step, D_800A8700;
extern void CasinoGame4LoadVram0(void), CasinoGame4LoadVram1(void);
extern void CasinoGame4LoadSound(void), func_8008BD08(void), func_8008BFC4(void);
extern void func_8008AA64(int, int), CasinoSpritesSetOn(int, int, int);

void func_80088818(void)
{
    int i;

    if (g_casino_timer == 0)
        CasinoGame4LoadVram0();
    if (g_casino_timer == 1)
        CasinoGame4LoadVram1();
    if (g_casino_timer == 2)
    {
        CasinoGame4LoadSound();
        func_8008BD08();
        func_8008BFC4();
        CasinoFade(0, 1000, 128, 128, 128, 16);
        for (i = 0; i < D_800A8700; i++)
            func_8008AA64((unsigned char)i, 0);
        CasinoSpritesSetOn(D_800A8700 * 25 + 68, (9 - D_800A8700) * 25, 0);
    }
    if (g_casino_timer >= 18)
    {
        g_casino_timer = -1;
        g_casino_step = 18;
    }
}
