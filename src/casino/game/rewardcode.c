/* Persona 1 (JP) - dispatch casino reward codes and item ranges. */
#include <decomp/types.h>
extern void func_8008CF68(u_short, u_short);
extern void func_8008CFC0(u_short);
extern char D_800A6748[];
extern u8 reward_column __asm__("D_800A8700+0x1");
extern u8 reward_row __asm__("D_800A8700+0x2");
extern u8 D_800A878C;
void func_8008CE0C(void)
{
    u_short code = *(u_short *)(D_800A6748 + 2 + reward_column * 0x42 + reward_row * 0x254);
    u8 count;
    switch (code) {
    case 0x200:
        func_8008CF68(0x24, 0x55);
        break;
    case 0x201:
        func_8008CFC0(0x142);
        func_8008CFC0(0x152);
        func_8008CFC0(0x162);
        func_8008CFC0(0x172);
        break;
    case 0x202:
        count = D_800A878C + 10;
        D_800A878C = count;
        if (count >= 99) D_800A878C = 99;
        break;
    case 0x203:
        func_8008CF68(1, 14);
        func_8008CFC0(16);
        break;
    case 0x204:
        func_8008CF68(0x56, 0x65);
        break;
    case 0x205:
        func_8008CF68(0x13, 0x19);
        break;
    case 0x206:
        D_800A878C = 99;
        break;
    case 0x207:
        func_8008CF68(0x11, 0x22);
        break;
    default:
        func_8008CFC0(code);
        break;
    }
}
