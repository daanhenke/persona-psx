#include <persona/casino/casino.h>
#include <libsnd.h>

extern unsigned char D_800B4464[4][3], D_800A8704;
extern short D_800AF9E8[];
extern void func_8008AEE0(int, int, signed char, short);
extern void func_8008AFF0(int, int, signed char, short);

void func_8008AD00(unsigned char target[4][3], short speed)
{
    int row, column;
    signed char complete = 0, difference, step;

    for (row = 0; row < 4; row++)
        for (column = 0; column < 3; column++)
        {
            difference = target[row][column] - D_800B4464[row][column];
            if (difference)
            {
                SsSeqStop(g_casino_seqs[10]);
                SsSeqPlay(g_casino_seqs[10], 1, 1);
                if (difference < 0)
                {
                    step = -1;
                    D_800B4464[row][column]--;
                }
                else
                {
                    step = 1;
                    D_800B4464[row][column]++;
                }
                if (row / 2 == 0)
                    func_8008AEE0(row, column, step, speed);
                else
                    func_8008AFF0(row, column, step, speed);
            }
            else
                complete++;
        }
    if (complete == 12)
        D_800A8704 = 1;
    else
        D_800AF9E8[104] = 2048 / speed;
}
