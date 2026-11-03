/* Persona JP1 casino animation and confirmation state. */
extern struct
{
    char pad[360];
    struct
    {
        char pad[16];
        unsigned char active;
    } pal;
} D_80099B10;
extern unsigned char g_grid_select_palette_on;
extern unsigned char D_800A8700[], D_800A8820[][3][3], D_800AFC00[3][3], g_casino_step;
extern signed char D_800AF73C[], D_800AF738[];
extern unsigned int g_casino_pad_trig;
extern int g_casino_timer;
extern short g_casino_seqs[], D_800AF9E8[];
extern void *D_800A2BC8[];
extern void CasinoStartPalAnim(void *), func_8008BB84(void), func_8008B4A0(void), SsSeqStop(short),
    SsSeqPlay(short, char, short), CasinoTween(void *, short, short, short, short, short),
    CasinoStartAnim(void *, int, int, int, int, int, int, int, int, int, int);
void func_80088E64(void)
{
    int i;
    if (!g_grid_select_palette_on)
        CasinoStartPalAnim(&D_80099B10.pal);
    if (g_casino_pad_trig & 16)
        D_800AF73C[0]++;
    if (g_casino_pad_trig & 64)
        D_800AF73C[1]++;
    if (g_casino_pad_trig & 128)
        D_800AF738[3]++;
    if (g_casino_pad_trig & 32)
        D_800AF738[2]++;
    if (g_casino_pad_trig & 8 || g_casino_pad_trig & 2)
        D_800AF73C[2]++;
    func_8008BB84();
    func_8008B4A0();
    if (D_800AF73C[2] && !D_800AF73C[0] && !D_800AF73C[1] && !D_800AF738[2] && !D_800AF738[3])
    {
        SsSeqStop(g_casino_seqs[4]);
        SsSeqPlay(g_casino_seqs[4], 1, 1);
        for (i = 0; i < 3; i++)
        {
            D_800A8820[D_800A8700[1]][i][0] = D_800AFC00[i][0];
            D_800A8820[D_800A8700[1]][i][1] = D_800AFC00[i][1];
            D_800A8820[D_800A8700[1]][i][2] = D_800AFC00[i][2];
        }
        CasinoTween(D_800A2BC8[56], 24, 24, -48, -48, 12);
        CasinoStartAnim(D_800A2BC8[851], 16, 2048, 0, 0, 0, 0, 0, 0, 0, 0);
        g_casino_step = 113;
        D_800AF73C[2] = 0;
        g_casino_timer = -16;
        D_800AF9E8[104] = 0;
    }
}
