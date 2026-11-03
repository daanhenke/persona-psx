/* Persona JP1: matching casino state handler. */
extern int g_casino_timer;
extern unsigned char g_casino_step, D_80099A20[];
extern signed char D_800AF830[], D_800AF834[];
extern short g_casino_seqs[];
extern void SsSeqStop(short), SsSeqPlay(short, char, short), CasinoStartPalAnim(void *),
    CasinoSpritesSetOn(int, int, int), CasinoTexLayout(void *);
void func_80083934(void)
{
    unsigned char empty = 0;
    int i;
    if (g_casino_timer % 18 == 0)
        SsSeqStop(g_casino_seqs[11]);
    SsSeqPlay(g_casino_seqs[11], 1, 1);
    if (!D_80099A20[16])
        CasinoStartPalAnim(D_80099A20);
    if (!g_casino_timer)
    {
        CasinoSpritesSetOn(461, 32, 1);
        CasinoSpritesSetOn(500, 16, 0);
        for (i = 5; i < 21; i++)
            CasinoTexLayout(((void **)(D_800AF834 + 8))[21 + i]);
    }
    if (D_800AF830[0] == 3)
    {
        for (i = 0; i < 5; i++)
        {
            if (D_800AF830[180 + i] != 3)
            {
                if (D_800AF830[180 + i])
                {
                    if (D_800AF834[323 + i] == D_800AF834[344 + i])
                    {
                        *(short *)(D_800AF830 + 222 + i * 2) = 0;
                        D_800AF830[180 + i] = 3;
                    }
                }
                else
                    empty++;
            }
        }
        if (empty == 5)
            D_800AF830[0] = 5;
    }
    if (D_800AF830[0] == 5)
    {
        SsSeqStop(g_casino_seqs[11]);
        g_casino_timer = -1;
        g_casino_step = 20;
    }
}
