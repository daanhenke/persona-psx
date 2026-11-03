/* Update the reward digits and start their quarter-turn reveal. */
#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <persona/casino/casino.h>

extern CasinoModel reward_models[3] __asm__("D_800A2BC8+0x3454");
extern void CasinoStartAnim(CasinoObj *, short, short, short, short, long, long, long, long, long, long);
extern void CasinoTexObj(CasinoModel*);

void func_800913FC(u8 number)
{
    CasinoModel* model;
    if ((u32)number < 10)
    {
        *reward_models[0].tex_idx = number;
        CasinoStartAnim(reward_models[0].obj, 16, 1024, 0, 0, 0, 0, 0, 0, 0, 0);
        CasinoTexObj(&reward_models[0]);
    }
    else
    {
        model = &reward_models[2];
        *reward_models[2].tex_idx = (u8)((u32)number % 10);
        *reward_models[1].tex_idx = (u8)((u32)number / 10);
        CasinoTexObj(model);
        CasinoTexObj(model - 1);
        CasinoStartAnim(reward_models[1].obj, 16, 1024, 0, 0, 0, 0, 0, 0, 0, 0);
        CasinoStartAnim(reward_models[2].obj, 16, 1024, 0, 0, 0, 0, 0, 0, 0, 0);
    }
}
