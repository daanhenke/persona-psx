#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <persona/casino/casino.h>

extern char D_8009E8E8[];
extern CasinoLayout* D_800A1624;
extern CasinoLayout* D_800A1768;
extern CasinoLayout* D_800A17BC;
extern CasinoObj D_800A16F8;
extern void func_80087C98(void*);
extern void CasinoTween();
extern void CasinoStartAnim();

void func_800885C4(void)
{
    if (g_casino_pad_trig & 0x20)
    {
        g_casino_win = 0;
        func_80087C98(D_8009E8E8);
        CasinoTween(D_800A1768, 320, 0, 0, 0, 32);
        CasinoTween(D_800A17BC, -320, 0, 0, 0, 32);
        CasinoTween(D_800A1624, 320, 0, 0, 0, 32);
        CasinoStartAnim(&D_800A16F8, 32, 0, 0, 0, -320, 0, 0, 0, 0, 0);
        g_casino_step = 0x26;
        g_casino_timer = -1;
    }
    else if ((g_casino_pad_trig & 0x80) && (u32)g_casino_win < 10000000)
    {
        g_casino_step = 0x77;
        g_casino_timer = 32;
    }
}
