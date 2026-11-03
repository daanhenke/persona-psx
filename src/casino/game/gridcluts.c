extern unsigned char D_800A8700[];
extern signed char D_800A8820[][3][3], D_800A87E0[3][3];
extern void CasinoQueueCluts(int, int, unsigned short);

void func_8008D120(void)
{
    int i, j, k, base;
    unsigned short color = 0x7EA4;

    for (i = 0; i < D_800A8700[1] + 1; i++)
        for (j = 0, base = 68; j < 3; base += 5, j++)
            for (k = 0; k < 3; k++)
                if (D_800A8820[i][j][k] == D_800A87E0[j][k])
                    CasinoQueueCluts(i * 25 + base + k, 1, color);
}
