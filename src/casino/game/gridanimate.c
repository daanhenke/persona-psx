/* Advance each grid cell toward a by-value snapshot of the target grid. */
#include <persona/casino/casino.h>
#include <libsnd.h>
#include <persona/casino/grid.h>

extern CasinoModel models[] __asm__("D_800A2BC8+4560");
extern signed char D_800A9424[3][3];
extern void        CasinoQueueCluts();
extern void        CasinoStartAnim();
extern void        func_8008BA08(void);

static inline void Grow(short duration)
{
    unsigned char x = g_casino_cursor.x;
    unsigned char y = g_casino_cursor.y;
    func_8008BA08();
    CasinoStartAnim(models[x + y * 3].obj, 2048 / duration, 2048, 0, 0, 0, 0, 0, 0, 0, 0);
}
short func_8008CBF8(signed char cells[3][3], TargetGrid goal)
{
    short matched = 0;
    int   row, col;
    for (row = 0; row < 3; row++)
    {
        for (col = 0; col < 3; col++)
        {
            if (D_800A9424[col][row] == 1)
            {
                D_800A9424[col][row] = 0;
                CasinoQueueCluts((row * 3 + col) * 2 + 26, 2, 0x7e24);
            }
            if (cells[row][col] != goal.cells[row][col])
            {
                if (models[row * 3 + col].obj->busy == 0)
                {
                    SsSeqStop(g_casino_seqs[9]);
                    SsSeqPlay(g_casino_seqs[9], 1, 1);
                    cells[row][col]   = (cells[row][col] + 1) % 9;
                    g_casino_cursor.x = col;
                    g_casino_cursor.y = row;
                    Grow(128);
                }
            }
            else
                matched++;
        }
    }
    return matched;
}
