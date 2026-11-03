/* Persona 1 (JP) - restore the bet and dispatch a blackjack result. */
#include <decomp/types.h>
extern u_char D_800A8768, D_800B09F4, g_casino_bet;
extern int g_casino_timer;
extern void func_8007C514(int, int);
extern void func_8007C8E4(int, int);
extern void func_8007CA5C(int, int);

void func_8007C2B8(void)
{
    if (g_casino_timer == 0) {
        g_casino_bet = D_800B09F4;
    }
    switch (D_800A8768) {
    case 11: func_8007C514(0, 0x79); break;
    case 12: func_8007C8E4(0, 0x79); break;
    case 13: func_8007CA5C(0, 0x79); break;
    }
}
