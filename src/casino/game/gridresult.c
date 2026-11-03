/* Resolve a grid round and advance its rolling digit display. */
#include <persona/casino/casino.h>
#include <persona/casino/grid.h>
/* Typed views of the original byte tables for the by-value display calls. */
extern CasinoSymbolBlock grid_symbols __asm__("D_800AFC00");
extern CasinoDigitRows grid_digits __asm__("D_800B4758");
extern u_char D_800A8700[], D_800A8704;
extern u_char grid_player __asm__("D_800A8700+1");
extern short grid_ticks __asm__("D_800AF9E8+0xD0");
extern short D_800A2BC8;
extern void func_8008B10C(int, int), func_8008AD00(u_char [4][3], short);
extern u_char func_8008C9DC(void);

void func_8008915C(void)
{
    if (g_casino_timer == 0) {
        D_800A8704 = 0;
        func_8008AAF8(grid_player, grid_symbols);
        D_800A8700[3] = func_8008C9DC();
        if (grid_player == 0) {
            func_8008B10C(1, 128);
            D_800A2BC8 = 16;
        } else D_800A2BC8 = 0;
    }
    if (g_casino_timer >= D_800A2BC8) {
        if (grid_ticks == 0) func_8008AD00(grid_digits.digit, 128);
        grid_ticks--;
    }
    if (D_800A8704 == 1) {
        if (D_800A8700[3] == 1) {
            g_casino_step = 114;
            g_casino_timer = -1;
        } else if (D_800A8700[3] == 0) {
            g_casino_step = 116;
            g_casino_timer = -1;
        }
        func_8008ABD8(grid_player, grid_digits);
        D_800A8704 = 0;
    }
}
