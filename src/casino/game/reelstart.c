/* Persona 1 (JP): initialize reel timing and random positions. */
extern unsigned int g_casino_timer;
extern signed char  g_casino_step;
extern short        g_casino_seqs[];
extern signed char  D_800AF830[], D_800AF834[];
extern char         D_800A24D4[];
extern void         SsSeqStop(short), SsSeqPlay(short, char, short), CasinoBuildLayouts(void*, int);
extern int          rand(void);
void                func_80083224(void)
{
    int          i;
    signed char* p;
    if (g_casino_timer == 0)
    {
        SsSeqStop(g_casino_seqs[11]);
        SsSeqPlay(g_casino_seqs[11], 1, 1);
        i = 20;
        p = D_800AF834 + 258;
        do
        {
            *(short*)p = 0;
            i--;
            p -= 2;
        } while (i >= 0);
        D_800AF830[0] = 4;
        D_800AF830[1] = 4;
        D_800AF830[2] = 4;
        D_800AF830[3] = 4;
        D_800AF834[0] = 4;
    }
    if (g_casino_timer < 21)
    {
        D_800AF834[176 + g_casino_timer] = 1;
        D_800AF834[323 + g_casino_timer] = rand() % 28 - 3;
        if (D_800AF834[323 + g_casino_timer] < 0)
            D_800AF834[323 + g_casino_timer] += 28;
    }
    if ((int)g_casino_timer >= 22)
    {
        g_casino_timer = -16;
        g_casino_step  = 115;
        CasinoBuildLayouts(D_800A24D4, 0);
    }
}
