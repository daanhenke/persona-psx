/* Persona 1 (JP) - resolve a reel result on entry and save its rank. */
#include <decomp/types.h>
extern u_char D_800A85F8[], D_800AF834[], D_800AF9E4[];
extern u_char g_casino_bet;
extern u_char result_slot __asm__("D_800AF9E4+0x2");
extern u_char result_rank __asm__("D_800A85F8+0x27");
extern s8 g_casino_step;
extern int g_casino_timer;
extern void func_800870B4(u_char *, s8 *, s8 *);
extern void func_8008749C(u_char *, u_char);
extern void func_80087B08(u_char *);

void func_80083B1C(void)
{
    u_char *symbols;
    u_char *rank;
    if (g_casino_timer == 0) {
        if (g_casino_bet < 5) {
            result_slot = g_casino_bet;
        } else {
            result_slot = g_casino_bet * 2 - 4;
        }
        symbols = D_800AF834 + 0x197;
        func_800870B4(symbols, (s8 *)symbols - 0x3F, (s8 *)symbols - 0x15);
        func_8008749C(symbols, g_casino_bet);
        func_80087B08(symbols);
        rank = &result_rank;
        D_800A85F8[result_slot] = *rank;
        D_800A85F8[13 + result_slot] = *rank;
        g_casino_timer = -1;
        g_casino_step = 0x74;
    }
}
