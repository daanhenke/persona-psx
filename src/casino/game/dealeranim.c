/* Persona 1 (JP) - play the dealer animation and refresh its card total. */
#include <decomp/types.h>
extern char g_casino_objs[], g_bj_dealer[], g_bj_dealer_total[];
extern s8 D_800A8C7C;
extern int g_casino_timer;
extern void CasinoStartAnim();
extern void func_8008205C(void *, void *);

void func_8007D4B8(void)
{
    if (g_casino_timer == 0) {
        CasinoStartAnim(g_casino_objs + 0x150, 32, 0, 0x800, 0, 0, 0, 0, 0, 0, 0);
    }
    if (g_casino_timer >= 33) {
        g_casino_timer = 0;
        func_8008205C(g_bj_dealer, g_bj_dealer_total);
        D_800A8C7C = 20;
    }
}
