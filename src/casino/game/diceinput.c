/* Persona JP1 casino input and hand state. */
extern signed char D_800AF738[], D_800AF73C[], D_800A8C72[];
extern unsigned int g_casino_pad, g_casino_pad_trig;
extern unsigned char g_casino_quit, g_casino_bet_done, D_800AAE9C[];
extern int g_casino_timer;
extern short g_casino_seqs[], g_casino_main_vab;
extern void CasinoKeyRepeat(signed char *, signed char *, unsigned int, int),
    CasinoPlaySeq(short *, unsigned int, short), func_80090248(void), func_80090330(void),
    func_80090448(void), func_800904FC(void);
void func_8008FF1C(void)
{
    CasinoKeyRepeat(D_800AF738 + 2, D_800A8C72, g_casino_pad & 32, 4);
    CasinoKeyRepeat(D_800AF738 + 3, D_800A8C72 + 1, g_casino_pad & 128, 4);
    CasinoKeyRepeat(D_800AF738 + 4, D_800A8C72 + 2, g_casino_pad & 16, 4);
    CasinoKeyRepeat(D_800AF738 + 5, D_800A8C72 + 3, g_casino_pad_trig & 64, 4);
    CasinoKeyRepeat(D_800AF738, D_800A8C72 - 2, g_casino_pad_trig & 4, 4);
    CasinoKeyRepeat(D_800AF738 + 1, D_800A8C72 - 1, g_casino_pad_trig & 1, 4);
    CasinoKeyRepeat(D_800AF738 + 6, D_800A8C72 + 4, g_casino_pad_trig & 8, 4);
    CasinoKeyRepeat(D_800AF738 + 7, D_800A8C72 + 5, g_casino_pad_trig & 2, 4);
    if (!D_800AF738[5] && !D_800AF738[3])
    {
        if (D_800AF738[6] || D_800AF738[7])
        {
            if (!g_casino_quit)
            {
                if (D_800AAE9C[0])
                    g_casino_bet_done = 1;
                else
                    CasinoPlaySeq(g_casino_seqs, 0x13B1F8, g_casino_main_vab);
            }
            D_800AF73C[2] = D_800AF73C[3] = 0;
        }
        else if ((D_800AF738[0] || D_800AF738[1]) && g_casino_timer >= 16)
        {
            if (!g_casino_bet_done)
                g_casino_quit = 1;
            D_800AF738[0] = D_800AF738[1] = 0;
        }
    }
    func_80090248();
    func_80090330();
    func_80090448();
    func_800904FC();
    D_800AF73C[0] = D_800AF73C[1] = 0;
    D_800AF738[2] = D_800AF738[3] = 0;
    D_800AF73C[2] = D_800AF73C[3] = 0;
    D_800AF738[0] = D_800AF738[1] = 0;
}
