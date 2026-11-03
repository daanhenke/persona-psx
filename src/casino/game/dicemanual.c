extern unsigned int g_casino_pad;
extern short D_800B0720[];
extern int D_800B0724[];
void D_8008FCDC(void)
{
    if (g_casino_pad & 32)
        D_800B0720[1] += 11;
    if (g_casino_pad & 128)
        D_800B0720[1] += -11;
    if (g_casino_pad & 16)
        D_800B0720[0] += 11;
    if (g_casino_pad & 64)
        D_800B0720[0] += -11;
    if (g_casino_pad & 8192)
        D_800B0724[1] += 2;
    if (g_casino_pad & 32768)
        D_800B0724[1] += -2;
    if (g_casino_pad & 4096)
        D_800B0724[2] += -2;
    if (g_casino_pad & 16384)
        D_800B0724[2] += 2;
    if (g_casino_pad & 4)
        D_800B0724[3] += 2;
    if (g_casino_pad & 8)
        D_800B0724[3] += -2;
}
