#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <persona/casino/casino.h>
#include <persona/casino/grid.h>
extern CasinoLayout* digit_rows[][2] __asm__("D_800A2BC8+0xB68");
extern void CasinoTexLayout(CasinoLayout*);

void func_8008ABD8(u8 index, CasinoDigitRows rows)
{
    int i;
    for (i = 0; i < 3; i++)
    {
        digit_rows[index][0]->tex_idx[3 + 5 * i] = (s8)rows.digit[0][i] + 47;
        digit_rows[index][0]->tex_idx[4 + 5 * i] = (s8)rows.digit[1][i] + 52;
        digit_rows[index][0]->tex_idx[15 + i] = (s8)rows.digit[2][i] + 57;
        digit_rows[index][0]->tex_idx[20 + i] = (s8)rows.digit[3][i] + 62;
    }
    CasinoTexLayout(digit_rows[index][0]);
}
