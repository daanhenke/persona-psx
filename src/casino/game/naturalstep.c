/* Persona 1 (JP) - blackjack natural-hand state transitions. */
#include <decomp/types.h>

extern u_char D_800A8674;
extern u_char g_bj_natural[];
extern u_char D_800A8768[];
extern u_char g_bj_play_step;
extern u_char D_800A8C7C;
extern int g_casino_timer;
extern void func_8007FBD8(int player);

void func_8007C080(void)
{
    if (g_bj_natural[D_800A8674] == 10) {
        func_8007FBD8(D_800A8674);
    }
    D_800A8768[D_800A8674] = 0x71;
    g_bj_play_step = 0x21;
}

void func_8007C0E4(void)
{
    if ((u_int)(g_bj_natural[0] - 1) < 2
        || (u_int)(g_bj_natural[1] - 1) < 2) {
        g_bj_play_step = 0x74;
        D_800A8C7C = 0x12;
        g_casino_timer = -1;
    } else {
        g_bj_play_step = 0x25;
    }
}
