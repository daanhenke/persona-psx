/* Persona 1 (JP) - advance the dealer state when its timer expires. */
#include <decomp/types.h>

extern int g_casino_timer;
extern u_char g_bj_dealer_bj;
extern u_char D_800A8C7C;

void func_8007D47C(void)
{
    if (g_casino_timer == 0) {
        g_bj_dealer_bj = 1;
        g_casino_timer = -1;
        D_800A8C7C = 0x35;
    }
}
