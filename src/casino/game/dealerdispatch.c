#include <persona/casino/blackjack.h>

extern unsigned char D_800A8C7C;
extern void func_8007D47C(void), func_8007D4B8(void), func_8007D554(void);
extern void func_8007D714(void), func_8007D9F4(void);

void func_8007D3CC(unsigned char *play_step)
{
    switch (D_800A8C7C) {
    case 0x12: func_8007D47C(); break;
    case 0x35: func_8007D4B8(); break;
    case 0x14: func_8007D554(); break;
    case 0x70: func_8007D714(); break;
    case 0x71: func_8007D9F4(); break;
    }
}
