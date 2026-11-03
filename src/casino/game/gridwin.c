extern int g_casino_timer;
extern unsigned int g_casino_pad_trig;
extern unsigned char D_800A8700[], g_casino_wrapped, g_casino_step, g_grid_win_palette_on;
extern short g_casino_seqs[], g_casino_vab, D_800A86FC;
extern short D_80099C90[];
extern void *D_800A2BC8[];
extern unsigned short GetClut(int, int);
extern void func_8008D120(void), CasinoPlaySeq(short *, int, short),
    CasinoQueueCluts(int, int, unsigned short), CasinoStartPalAnim(void *), SsSeqStop(short),
    SsSeqPlay(short, char, short), SsSeqSetVol(short, short, short),
    CasinoSpritesSetOn(int, int, int), func_8008B428(void), CasinoCloseSeq(short *),
    func_8008A974(int, int), CasinoTween(void *, short, short, short, short, short);
void func_800895EC(void)
{
    int x, y;
    if (!g_casino_timer)
    {
        func_8008D120();
        if (D_800A8700[1] + 1 < 4)
            CasinoPlaySeq(&g_casino_seqs[1], 0x12ef6c, g_casino_vab);
        else
            CasinoPlaySeq(&g_casino_seqs[2], 0x12efa8, g_casino_vab);
    }
    if (g_casino_timer == 32)
    {
        CasinoQueueCluts(25, 1, GetClut(D_80099C90[4], D_80099C90[5]));
        CasinoPlaySeq(&g_casino_seqs[3], 0x12e89c, g_casino_vab);
    }
    if (g_casino_timer >= 32)
    {
        if (!g_grid_win_palette_on)
            CasinoStartPalAnim(D_80099C90);
        if (g_casino_pad_trig & 2)
            CasinoPlaySeq(&g_casino_seqs[1], 0x12ef6c, g_casino_vab);
        if (g_casino_pad_trig & 1)
            CasinoPlaySeq(&g_casino_seqs[2], 0x12efa8, g_casino_vab);
        if (g_casino_pad_trig & 4 || g_casino_pad_trig & 8)
        {
            SsSeqStop(g_casino_seqs[3]);
            SsSeqPlay(g_casino_seqs[3], 1, 1);
        }
        if (!(g_casino_timer % 24))
            CasinoSpritesSetOn(16, 1, (((g_casino_timer / 24) & 1) ^ 1));
        if (!(g_casino_timer & 7))
            CasinoSpritesSetOn(293, 40, (g_casino_timer / 8) % 2);
        if (!(g_casino_timer & 15))
        {
            D_800A86FC = (0x155 >> ((g_casino_timer / 16) % 2)) & 0x1ff;
            func_8008B428();
        }
        if (g_casino_pad_trig & 8192 || g_casino_pad_trig & 32768 || g_casino_pad_trig & 4096 ||
            g_casino_pad_trig & 16384 || g_casino_pad_trig & 32 || g_casino_pad_trig & 128 ||
            g_casino_pad_trig & 16 || g_casino_pad_trig & 64)
        {
            CasinoCloseSeq(&g_casino_seqs[3]);
            if (g_casino_wrapped)
                SsSeqStop(g_casino_seqs[15]);
            SsSeqPlay(g_casino_seqs[15], 1, 1);
            SsSeqSetVol(g_casino_seqs[15], 127, 127);
            for (y = 0; y < 3; y++)
                for (x = 0; x < 3; x++)
                    func_8008A974(x, y);
            CasinoSpritesSetOn(16, 1, 1);
            CasinoTween(D_800A2BC8[217], -128, 0, 0, 0, 32);
            CasinoSpritesSetOn(293, 40, 1);
            CasinoQueueCluts(25, 1, 0x7ca4);
            g_casino_step = 120;
            g_casino_timer = -1;
            D_800A8700[1]++;
        }
    }
}
