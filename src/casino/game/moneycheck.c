/* Persona 1 (JP) - moneycheck helper. */
#include <decomp/types.h>

extern u_char D_800A8674, g_casino_bet;
extern u_int g_casino_money;
extern void func_8008174C(u_char);
extern void func_80081934(int);
void func_8007BA14(void) {
    int enough = g_casino_money >= g_casino_bet;
    func_8008174C(D_800A8674);
    func_80081934(enough);
}
