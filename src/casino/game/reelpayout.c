/* Persona JP1: matching casino state handler. */
extern unsigned char D_80099A08[], D_80099A38[], D_800AF9E4[], D_800AF834[], g_casino_wrapped,
    g_casino_step;
extern signed char D_800A85F8[];
extern unsigned int g_casino_win, g_casino_money;
extern int g_casino_timer, g_casino_frame;
extern short g_casino_seqs[];
extern void CasinoStartPalAnim(void *), func_800840C8(void), CasinoSplitDigits(unsigned int),
    CasinoPayStep(unsigned int *), SsSeqStop(short), SsSeqPlay(short, char, short),
    SsSeqSetVol(short, short, short);
void func_80083DA8(void)
{
    if (!D_80099A08[16])
        CasinoStartPalAnim(D_80099A08);
    if (!D_80099A38[16])
        CasinoStartPalAnim(D_80099A38);
    if (g_casino_timer == 0)
    {
        *(short *)(D_800AF834 + 218) = 0;
        D_800AF9E4[3] = 2;
    }
    if (D_800AF9E4[3] != 8)
    {
        if (D_800A85F8[39] > 0)
            func_800840C8();
        else
        {
            CasinoSplitDigits(g_casino_win);
            D_800AF9E4[3] = 8;
        }
    }
    if (D_800AF9E4[3] == 8)
    {
        if (g_casino_win)
        {
            if (!(g_casino_frame & 1))
                CasinoPayStep(&g_casino_win);
            if (g_casino_money >= 99999999)
            {
                g_casino_win = 0;
                g_casino_money = 99999999;
            }
        }
        if (D_800AF9E4[3] == 8 && !g_casino_win)
        {
            if (g_casino_wrapped)
                SsSeqStop(g_casino_seqs[15]);
            SsSeqPlay(g_casino_seqs[15], 1, 1);
            SsSeqSetVol(g_casino_seqs[15], 127, 127);
            g_casino_timer = -1;
            g_casino_step = 118;
        }
    }
}
