#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <persona/casino/casino.h>

extern u8 D_800B4464[][3];
extern CasinoModel horizontal_models[] __asm__("D_800A2BC8+0x16C8");
extern void CasinoTexObj(CasinoModel*);
extern void CasinoStartAnim();

void func_8008AFF0(s32 row, s32 column, s32 direction, s16 speed)
{
    s32 even;
    s32 index = (row - 2) * 3 + column;
    u8 *base = (u8*)D_800B4464;
    u8 *entry = base + row * 3 + column;
    even = !(*entry % 2);
    horizontal_models[index].tex_idx[even] = (s8)*entry * 2 + even;
    CasinoTexObj(&horizontal_models[index]);
    CasinoStartAnim(horizontal_models[index].obj, 2048 / speed, (s8)direction * 2048, 0, 0, 0, 0, 0, 0, 0, 0);
}
