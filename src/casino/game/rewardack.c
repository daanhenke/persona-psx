/* Reward presentation and acknowledgment state. */
extern int g_casino_timer;
extern unsigned int g_casino_pad_trig;
extern unsigned char g_casino_wrapped, g_casino_step, D_800A9724[];
extern short g_casino_seqs[], g_casino_main_vab;
extern void *D_800A2BC8[];
extern void func_800915FC(void), CasinoSpritesSetOn(int, int, int),
    CasinoPlaySeq(short *, int, short), func_80091544(unsigned char),
    CasinoTween(void *, short, short, short, short, short), SsSeqSetVol(short, short, short),
    SsSeqStop(short), SsSeqPlay(short, char, short);
void func_8008DD44(void)
{
    if (g_casino_timer > 0)
    {
        func_800915FC();
        if (!(g_casino_timer & 7))
            CasinoSpritesSetOn(35, 8, 0);
        else if (g_casino_timer % 8 == 4)
            CasinoSpritesSetOn(35, 8, 1);
    }
    if (!g_casino_timer)
    {
        CasinoPlaySeq(g_casino_seqs, 0x13B06C, g_casino_main_vab);
        func_80091544(D_800A9724[2]);
        CasinoTween(D_800A2BC8[1820], 0, -80, 0, 0, 16);
        CasinoTween(D_800A2BC8[3023], 0, -208, 0, 0, 16);
    }
    if (g_casino_timer == 160)
    {
        SsSeqSetVol(g_casino_seqs[15], 127, 127);
        if (g_casino_wrapped)
            SsSeqStop(g_casino_seqs[15]);
        SsSeqPlay(g_casino_seqs[15], 1, 1);
    }
    if (g_casino_timer >= 160)
    {
        if (g_casino_pad_trig & 8192 || g_casino_pad_trig & 32768 || g_casino_pad_trig & 4096 ||
            g_casino_pad_trig & 16384 || g_casino_pad_trig & 32 || g_casino_pad_trig & 128 ||
            g_casino_pad_trig & 16 || g_casino_pad_trig & 64 || g_casino_pad_trig & 4 ||
            g_casino_pad_trig & 1 || g_casino_pad_trig & 8 || g_casino_pad_trig & 2)
        {
            g_casino_step = 116;
            g_casino_timer = -1;
        }
    }
}
