/* Finish a stopped row after its settle timer, then refresh its four textures. */
#include <persona/casino/casino.h>
extern s8 row_work[] __asm__("D_800AF830");
extern CasinoLayout *D_800A24B0[][2];
extern u_char *D_800B3E28[];
extern void CasinoSpritesSetOn(short, short, int);
extern void CasinoTexLayout(CasinoLayout *);
extern void func_80087124(s8, u_char *, s8 *, s8 *);
void func_80085B74(s8 row)
{
    int base;
    int i;
    int end;
    CasinoLayout **layout;
    u_char **table;
    u_char *map;
    int digit;
    if (row_work[row + 1] == 5)
    {
        if (row_work[row + 5] >= 17)
        {
            func_80087124(row, (u_char *)(row_work + 411), row_work + 348, row_work + 390);
            do { base = row * 4; } while(0);
            CasinoSpritesSetOn(base + 500, 4, 1);
            CasinoSpritesSetOn(row * 8 + 461, 8, 0);
            i = base + 5;

            if (i < base + 9)
            {
                end = base + 9;
                layout = D_800A24B0[i];
                { unsigned long base_table=(unsigned long)D_800B3E28; table=(u_char**)(base_table+i*sizeof(*table)); }
                do
                {
                    map = *table;
                    table++;
                    digit = row_work[base + 395];
                    base++;
                    (*layout)->tex_idx[0] = map[digit];
                    i++;
                    CasinoTexLayout(*layout);
                    layout += 2;
                } while (i < end);
            }
            row_work[row + 1] = 6;
        }
    }
    row_work[row + 5]++;
}
