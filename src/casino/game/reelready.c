/* Persona 1 (JP) - advance a reel group after its entries finish. */
#include <decomp/types.h>

extern s8 D_800AF830[];
extern s8 D_800AF834[];

void func_80085A7C(s8 group)
{
    s8 empty = 0;
    int i;

    if (D_800AF830[1 + group] == 3) {
        for (i = group * 4 + 5; i < group * 4 + 9; i++) {
            if (D_800AF834[0xB0 + i] != 3) {
                if (D_800AF834[0xB0 + i] != 0) {
                    if (D_800AF834[0x143 + i] == D_800AF834[0x158 + i]) {
                        D_800AF834[0xB0 + i] = 3;
                        *(s16 *)(D_800AF834 + 0xDA + i * 2) = 0;
                    }
                } else {
                    empty++;
                }
            }
        }
        if (empty == 4) {
            D_800AF830[1 + group] = 5;
            D_800AF834[1 + group] = 0;
        }
    }
}
