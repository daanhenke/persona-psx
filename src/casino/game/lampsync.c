#include <persona/casino/casino.h>

extern CasinoObj *main_lamp __asm__("D_800A2BC8+0x2C14");
extern CasinoObj *left_lamp __asm__("D_800A2BC8+0x3268");
extern CasinoObj *right_lamp __asm__("D_800A2BC8+0x32F0");
extern void CasinoStartAnim();

void func_80090C58(void)
{
    int rotation;

    if (CASINO_LAMPS.b4 != CASINO_LAMPS.b5)
    {
        rotation = -2048;
        if (CASINO_LAMPS.b4)
        {
            rotation = 2048;
        }
        CasinoStartAnim(main_lamp, 32, rotation, 0, 0, 0, 0, 0, 0, 0, 0);
        CASINO_LAMPS.b5 = CASINO_LAMPS.b4;
    }
    if (CASINO_LAMPS.b0 != CASINO_LAMPS.b1)
    {
        CasinoStartAnim(left_lamp, 16, 2048, 0, 0, 0, 0, 0, 0, 0, 0);
        CASINO_LAMPS.b1 = CASINO_LAMPS.b0;
    }
    if (CASINO_LAMPS.b2 != CASINO_LAMPS.b3)
    {
        CasinoStartAnim(right_lamp, 16, 2048, 0, 0, 0, 0, 0, 0, 0, 0);
        CASINO_LAMPS.b3 = CASINO_LAMPS.b2;
    }
}
