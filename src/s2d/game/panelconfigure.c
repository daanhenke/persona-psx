/* Persona JP1: configure one of the 40-byte map gradient-panel records. */
extern short D_800B5E28[][20];
extern short D_800B5E2C[][20];
extern short D_800B5E30[][20];

void func_8009994C(int x, int y, int width, int height,
                   int alignment, int index, int rate)
{
    D_800B5E28[index][0] = 1;
    D_800B5E28[index][1] = alignment * 2;
    D_800B5E2C[index][0] = x - 256;
    D_800B5E2C[index][1] = y - 120;
    D_800B5E30[index][0] = width;
    D_800B5E30[index][1] = height;
    *(int *)(D_800B5E30[index] + 14) = rate;
}
