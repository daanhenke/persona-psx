/* Persona 1 (JP) - clear the reel cursor inputs and twelve-entry grids. */
#include <decomp/types.h>
extern char g_casino_key_fire[], g_casino_key_hold[];
extern s8 D_800B4464[], D_800B4758[];
extern u_short D_800A86FC[];
extern s8 D_800A8700[], D_800A8704;
extern int g_casino_timer;
extern s8 g_casino_step;
extern void CasinoCursorClear(void *);

void func_80088954(void)
{
    int row;
    s8 *first;
    s8 *second;
    s8 *a;
    s8 *b;
    if (g_casino_timer >= 0) {
        CasinoCursorClear(g_casino_key_fire);
        CasinoCursorClear(g_casino_key_hold);
        row = 0;
        second = D_800B4758;
        first = D_800B4464;
        do {
            b = second;
            a = first;
            do {
                *a = 0;
                *b = 0;
                b++;
                a++;
            } while ((int)b < (int)(second + 3));
            second += 3;
            row++;
            first += 3;
        } while (row < 4);
        D_800A86FC[0] = 1;
        D_800A86FC[1] = 1;
        g_casino_timer = -1;
        D_800A8700[3] = 0;
        D_800A8704 = 0;
        D_800A8700[1] = 0;
        g_casino_step = 38;
    }
}
