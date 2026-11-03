#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <persona/casino/casino.h>

extern CasinoLayout* tween_left __asm__("D_800A2BC8+0x2C68");
extern CasinoLayout* tween_middle __asm__("D_800A2BC8+0x2CC4");
extern CasinoLayout* tween_right __asm__("D_800A2BC8+0x2D20");
extern CasinoLayout* tween_frame __asm__("D_800A2BC8+0x2ADC");
extern u8 tween_state __asm__("D_800AAE98+0x2");
extern u8 tween_previous __asm__("D_800AAE98+0x3");
extern void CasinoTween();

void func_800906D0(void)
{
    if (tween_state != tween_previous)
    {
        if (tween_state == 0)
        {
            CasinoTween(tween_left, -8, -8, 22, 22, 8);
            CasinoTween(tween_middle, -8, -8, 22, 22, 8);
            CasinoTween(tween_right, -8, -8, 22, 22, 8);
            CasinoTween(tween_frame, -8, -8, 22, 22, 8);
        }
        else
        {
            CasinoTween(tween_left, 8, 8, -22, -22, 8);
            CasinoTween(tween_middle, 8, 8, -22, -22, 8);
            CasinoTween(tween_right, 8, 8, -22, -22, 8);
            CasinoTween(tween_frame, 8, 8, -22, -22, 8);
        }
        tween_previous = tween_state;
    }
}
