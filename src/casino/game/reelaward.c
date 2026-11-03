/* Persona JP1: matching casino state handler. */
extern unsigned char D_800999D8[], D_800999F0[], D_80099A08[], D_80099A38[], D_800AF9E4[],
    D_800AF9E8[], D_800AF834[], g_casino_step;
extern short g_casino_seqs[];
extern int g_casino_timer;
extern void CasinoStartPalAnim(void *), CasinoQueueCluts(int, int, unsigned short),
    CasinoSpritesSetOn(int, int, int), func_80084D50(signed char, int), func_80084584(void),
    SsSeqSetVol(short, short, short);
extern unsigned short GetClut(short, short);
void func_80083BFC(void)
{
    int i;
    if (!D_800999D8[16])
        CasinoStartPalAnim(D_800999D8);
    if (!D_800999F0[16])
        CasinoStartPalAnim(D_800999D8 + 24);
    if (!D_80099A08[16])
        CasinoStartPalAnim(D_80099A08);
    if (!D_80099A38[16])
        CasinoStartPalAnim(D_80099A38);
    if (g_casino_timer == 0)
    {
        CasinoQueueCluts(537, 21, GetClut(*(short *)(D_80099A08 + 8), *(short *)(D_80099A08 + 10)));
        D_800AF9E4[3] = 1;
        *(short *)(D_800AF834 + 218) = 0;
        D_800AF9E4[1] = 0;
        D_800AF9E4[0] = 0;
        D_800AF9E8[0] = 255;
        D_800AF9E8[1] = 255;
        D_800AF9E8[4] = 0;
        for (i = 0; i < 8; i++)
            func_80084D50(i, 0);
        SsSeqSetVol(g_casino_seqs[15], 64, 64);
    }
    if (g_casino_timer >= 0)
        func_80084584();
    if (D_800AF9E4[0] >= D_800AF9E4[2])
    {
        CasinoSpritesSetOn(537, 21, 0);
        g_casino_timer = -1;
        g_casino_step = 117;
    }
}
