/* Persona JP1 casino animation and confirmation state. */
extern struct
{
    char pad[8];
    short x, y;
    char pad2[4];
    unsigned char active;
} D_80099990;
extern unsigned char g_bj_result_palette_on;
extern unsigned char g_bj_natural[], g_casino_bet, D_800A8818[], g_bj_play_step;
extern char D_8009C274[];
extern short g_casino_seqs[], D_80099DC0[];
extern int g_casino_timer, g_casino_win;
extern unsigned int g_bj_jackpot, g_casino_pad_trig;
extern void CasinoStartPalAnim(void *), SsSeqSetVol(short, short, short),
    func_8007FFAC(unsigned char, int), func_8007F5F8(void *), CasinoSplitDigits(int),
    CasinoQueueCluts(int, int, unsigned short),
    CasinoStartAnim(void *, int, int, int, int, int, int, int, int, int, int);
extern unsigned short GetClut(short, short);
extern short SsIsEos(short, short);
void func_8007A39C(void)
{
    if (!g_bj_result_palette_on)
        CasinoStartPalAnim(&D_80099990);
    if (!g_casino_timer)
    {
        SsSeqSetVol(g_casino_seqs[15], 64, 64);
        func_8007FFAC(g_bj_natural[0], 0);
        func_8007F5F8(g_bj_natural);
        if (!g_bj_natural[0] && g_casino_bet >= 10)
            g_casino_win = (D_800A8818[0] + 1) * (int)(g_bj_jackpot / 100);
        else
            g_casino_win = g_casino_bet * D_80099DC0[g_bj_natural[0]];
        CasinoSplitDigits(g_casino_win);
        CasinoQueueCluts(g_bj_natural[0] + 748, 1, GetClut(D_80099990.x, D_80099990.y));
        CasinoQueueCluts(g_bj_natural[0] + 755, 1, GetClut(D_80099990.x, D_80099990.y));
        CasinoQueueCluts(875, 24, 0x7C64);
        CasinoStartAnim(D_8009C274, 32, 0, 0, 0, -320, 0, 0, 0, 0, 0);
        CasinoStartAnim(D_8009C274 + 16, 32, 0, 0, 0, 320, 0, 0, 0, 0, 0);
    }
    if (!SsIsEos(g_casino_seqs[0], 0) && g_casino_timer >= 0)
    {
        SsSeqSetVol(g_casino_seqs[15], 127, 127);
        if (g_casino_pad_trig & 8192 || g_casino_pad_trig & 32768 || g_casino_pad_trig & 4096 ||
            g_casino_pad_trig & 16384 || g_casino_pad_trig & 32 || g_casino_pad_trig & 128 ||
            g_casino_pad_trig & 16 || g_casino_pad_trig & 64)
        {
            g_bj_play_step = 19;
            g_casino_timer = -1;
        }
    }
}
