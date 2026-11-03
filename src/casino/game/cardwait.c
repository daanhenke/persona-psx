/* Persona 1 (JP) - advance after the blackjack result display delay. */
#include <decomp/types.h>
extern u_char g_bj_natural[];
extern s8 g_bj_play_step;
extern int g_casino_timer;
extern void func_8007B288(void);
extern void func_8007B4DC(void);

void func_8007B208(void)
{
    int waiting;
    func_8007B288();
    func_8007B4DC();
    if (g_bj_natural[0] != 1 || g_bj_natural[1] != g_bj_natural[0]) {
        waiting = g_casino_timer < 129;
    } else {
        waiting = g_casino_timer < 97;
    }
    if (!waiting) {
        g_bj_play_step = 33;
    }
}
