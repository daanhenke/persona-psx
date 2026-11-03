/* Persona JP1: restore reel palettes and slide back to the betting state. */
#include <decomp/types.h>

/* These calls retain the original default argument promotions. */
extern void CasinoQueueCluts();
extern void CasinoTween();
extern void CasinoStartAnim();
extern void *D_800A1624;
extern char D_800A16F8[];
extern int g_casino_timer;
extern signed char g_casino_bet, g_casino_step;

void func_80083F8C(void)
{
    short cluts[7] = {0x7CA9, 0x7D29, 0x7DA9, 0x7E29,
                      0x7EA9, 0x7F29, 0x7FA9};
    int i;

    /* The original copies seven entries but reads eight. The last read is
       incoming stack data; preserve it here for binary matching. A native
       port must resolve that behavior before treating this as safe C. */
    for (i = 0; i < 8; i++) {
        CasinoQueueCluts(i + 0x99, 1, (u_short)cluts[i]);
    }
    for (i = 0; i < 6; i++) {
        CasinoQueueCluts(i * 8 + 0xA0, 1, 0x7D65);
    }
    CasinoTween(D_800A1624, 0x140, 0, 0, 0, 0x20);
    CasinoStartAnim(D_800A16F8, 0x20, 0, 0, 0, -0x140, 0, 0, 0, 0, 0);
    g_casino_timer = -1;
    g_casino_bet = 0;
    g_casino_step = 0x12;
}
