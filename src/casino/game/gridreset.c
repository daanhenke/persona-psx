extern unsigned char D_800B0AF0, g_casino_spot, D_800A8700[], D_800A8704[], D_800A6748[],
    D_800A8820[9][3][3], D_800A9424[3][3], D_800AF73C[], D_800A87E0[3][3], D_800AFC00[3][3];
extern unsigned g_casino_lamps[];
void func_8008BFC4(void)
{
    int i, j;
    D_800B0AF0 = 255;
    D_800A8704[1] = 0;
    D_800A8700[2] = g_casino_spot + 23;
    g_casino_lamps[0] &= ~1;
    g_casino_lamps[0] &= ~4;
    D_800A8700[0] = D_800A6748[D_800A8700[2] * 596];
    for (i = 0; i < 3; i++)
        for (j = 0; j < 3; j++)
        {
            D_800AFC00[i][j] = D_800A87E0[i][j] = D_800AF73C[8 + i * 3 + j] = D_800A9424[i][j] = 0;
            D_800A8820[0][i][j] = D_800A8820[1][i][j] = D_800A8820[2][i][j] = D_800A8820[3][i][j] =
                D_800A8820[4][i][j] = D_800A8820[5][i][j] = D_800A8820[6][i][j] =
                    D_800A8820[7][i][j] = D_800A8820[8][i][j] = 0;
        }
}
