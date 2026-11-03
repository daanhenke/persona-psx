#include <persona/casino/casino.h>

extern int g_casino_timer;
extern unsigned char g_casino_step, D_800A2BC8[];
extern void CasinoGame5LoadVram0(void), CasinoGame5LoadVram1(void);
extern void CasinoGame5LoadVram2(void), CasinoGame5LoadSound(void);
extern void func_8008F2B8(void), func_80093178(void);
extern void func_8008F7E4(void), func_80090E9C(void);
extern void func_80090F58(unsigned char);
extern void func_80091154(unsigned char, unsigned char);

void func_8008D39C(void)
{
    int i;

    if (g_casino_timer == 0)
        CasinoGame5LoadVram0();
    if (g_casino_timer == 1)
        CasinoGame5LoadVram1();
    if (g_casino_timer == 2)
        CasinoGame5LoadVram2();
    if (g_casino_timer == 3) {
        CasinoGame5LoadSound();
        func_8008F2B8();
        func_80093178();
        func_8008F7E4();
        func_80090E9C();
        CasinoFade(0, 416, 128, 128, 128, 32);
        for (i = 0; i < 11; i++) {
            func_80090F58(i);
            func_80091154(i, D_800A2BC8[i + 4]);
        }
    }
    if (g_casino_timer == 19) {
        g_casino_timer = -1;
        g_casino_step = 18;
    }
}
