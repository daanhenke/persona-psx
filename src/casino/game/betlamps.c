/* Persona 1 (JP) - update the bet controls' lamp bits. */
#include <decomp/types.h>

extern u_char g_casino_bet_shown, g_casino_bet, g_casino_max_bet;
extern u_int g_casino_lamps;

void func_8007E570(void)
{
    if (!g_casino_bet_shown && g_casino_bet == 1 && !g_casino_max_bet) {
        u_int *lamps = &g_casino_lamps;
        *lamps |= 0x14;
    }
    if (g_casino_bet_shown && !g_casino_bet) {
        u_int *lamps = &g_casino_lamps;
        *lamps &= ~0x10;
        *lamps &= ~0x04;
    }
}
