#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <persona/casino/casino.h>

extern short grid_tick __asm__("D_800AF9E8+0xD0");
extern CasinoLayout *D_800A2BC8[];
extern void CasinoTween(CasinoLayout *, int, short, short, short, short);
extern void CasinoShowLayout(CasinoLayout *, unsigned char);
extern void CasinoSpritesSetOn(int, int, int);

void func_8008C16C(void)
{
    if ((unsigned short)grid_tick < 128)
        CasinoTween(D_800A2BC8[217], -1, 0, 0, 0, 1);
    switch (grid_tick)
    {
    case 144:
    case 176:
    case 208:
        CasinoSpritesSetOn(16, 1, 0);
        break;
    case 160:
    case 192:
    case 224:
        CasinoSpritesSetOn(16, 1, 1);
        break;
    case 368:
        CasinoShowLayout(D_800A2BC8[217], 1);
        grid_tick = -1;
        break;
    }
    if ((unsigned int)((unsigned short)grid_tick - 240) < 128)
        CasinoTween(D_800A2BC8[217], -1, 0, 0, 0, 1);
    grid_tick++;
}
