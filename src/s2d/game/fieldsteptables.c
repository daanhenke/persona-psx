/* Select the field movement step and signed direction bit pattern. */
extern unsigned short D_800B0D3C, D_800B0D40;

void func_8008BB14(int choice)
{
    unsigned short steps[3] = {8, 8, 16};
    unsigned short signs[3] = {65535, 1, 65535};
    D_800B0D3C = steps[choice];
    D_800B0D40 = signs[choice];
}
