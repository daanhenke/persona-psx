/* Persona 1 (JP) - move the blackjack pieces out and advance its state. */
#include <decomp/types.h>
extern char g_casino_objs[];
extern s8 g_bj_play_step;
extern int g_casino_timer;
extern void CasinoStartAnim();

void func_8007B12C(void)
{
    char *object = g_casino_objs + 0x180;
    CasinoStartAnim(object, 10, 0, 0, 0, -72, 0, 0, 0, 0, 0);
    CasinoStartAnim(object, 2, 0, 0, 0, -2, 0, 0, 0, 0, 0);
    CasinoStartAnim(object - 32, 10, 0, 0, 0, 80, 0, 0, 0, 0, 0);
    g_bj_play_step = 46;
    g_casino_timer = -1;
}
