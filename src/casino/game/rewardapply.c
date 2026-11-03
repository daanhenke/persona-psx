/* Persona 1 (JP) - apply a reward and cap the money total. */
#include <decomp/types.h>

extern u_char reward_kind __asm__("D_800A8700+0x1");
extern u_char D_800A878C;
extern u_int g_casino_money;

void func_8008D04C(void)
{
    switch (reward_kind) {
    case 0:
        g_casino_money += 1000000;
        if (g_casino_money > 99999999U) {
            g_casino_money = 99999999;
        }
        break;
    case 1:
        g_casino_money += 5000;
        if (g_casino_money > 99999999U) {
            g_casino_money = 99999999;
        }
        break;
    case 2:
        D_800A878C++;
        break;
    }
}
