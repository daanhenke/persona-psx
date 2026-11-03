#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <persona/casino/casino.h>

extern CasinoModel grid_models[9] __asm__("D_800A2BC8+0x11D0");
extern s8 grid_modes[3][3] __asm__("D_800AF73C+0x8");
extern u8 D_800AFC00[3][3];
extern s8 D_800A9424[3][3];
extern void CasinoTexObj(CasinoModel*);
extern void CasinoQueueCluts();

void func_8008BA08(void)
{
    s8 row = g_casino_cursor.y;
    s8 column = g_casino_cursor.x;
    s8 mode = grid_modes[row][column];
    if (mode == 0)
    {
        grid_models[row * 3 + column].tex_idx[1] = (s8)D_800AFC00[row][column] + 9;
    }
    else if (mode == 1)
    {
        grid_models[row * 3 + column].tex_idx[0] = (s8)D_800AFC00[row][column];
    }
    CasinoTexObj(&grid_models[row * 3 + column]);
    CasinoQueueCluts((row * 3 + column) * 2 + 26, 2, (D_800A9424[column][row] << 6) + 0x7E24);
    grid_modes[row][column] = grid_modes[row][column] == 0;
}
