/* Persona 1 (JP) - build the reel-result mask and update changed palettes. */
#include <decomp/types.h>
extern u_short D_800A86FC[];
extern s8 D_800AFC00[];
extern void func_8008B428(void);

void D_8008B37C(void)
{
    int row, col;
    s8 *start;
    D_800A86FC[0] = 0;
    for (row = 0; row < 3; row++) {
        start = D_800AFC00 + row * 3;
        for (col = 0; col < 3; col++) {
            D_800A86FC[0] |= 1 << start[col];
        }
    }
    if (D_800A86FC[0] != D_800A86FC[1]) {
        func_8008B428();
        D_800A86FC[1] = D_800A86FC[0];
    }
}
