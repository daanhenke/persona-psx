#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <persona/casino/casino.h>

extern u_char g_bj_card_obj;
extern u_char exit_flag __asm__("D_800B0AF8+0x1");
extern u_char next_state __asm__("D_800B0AF8+0x3");
extern void CasinoStartAnim();
extern void CasinoSpritesSetOn(int, int, int);

void func_80080E0C(void)
{
    CasinoObj* object;
    if (g_casino_timer == 0)
    {
        object = &g_casino_objs[21];
        CasinoStartAnim(object, 64, 0, 2048, 0, 0, 0, 0, 0, 0, 0);
        CasinoStartAnim(g_casino_objs + g_bj_card_obj, 32, 0, 0, 0, 320, 0, 0, 0, 0, 0);
    }
    if (g_casino_timer >= 33)
    {
        CasinoSpritesSetOn(0, 140, 1);
        CasinoSpritesSetOn(28, 2, 0);
        CasinoSpritesSetOn(38, 2, 0);
        exit_flag = 0;
        g_casino_timer = 8;
        g_casino_step = next_state;
    }
}
