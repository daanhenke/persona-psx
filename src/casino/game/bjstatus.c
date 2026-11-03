/* Persona JP1 casino input and hand state. */
extern unsigned char D_800A8768[], g_bj_natural[], D_800A8674, D_800A8818[], g_bj_count[],
    D_800AAEA8[];
extern char D_8009C42C[];
extern unsigned char func_80081F80(void *), func_80082034(unsigned char),
    func_8008230C(unsigned char *);
extern void CasinoQueueCluts(int, int, int),
    CasinoStartAnim(void *, int, int, int, int, int, int, int, int, int, int),
    func_8007FBD8(unsigned char);
void func_8007F960(unsigned char hand, void *cards, unsigned char *total)
{
    int special = 0;
    if (func_80081F80(cards) == 3)
    {
        D_800A8768[hand] = 113;
        g_bj_natural[hand] = 3;
    }
    if (!D_800A8818[D_800A8674])
    {
        if (func_80082034(g_bj_count[hand]) == 10)
        {
            g_bj_natural[hand] = 10;
            D_800AAEA8[4] = 1;
        }
        if (func_80082034(g_bj_count[hand]) == 9)
        {
            D_800A8768[hand] = 113;
            g_bj_natural[hand] = 9;
        }
        if (*total == 21)
        {
            if (g_bj_natural[hand] == 1)
                D_800A8768[hand] = 113;
            else if (g_bj_natural[hand] == 10)
            {
                D_800A8768[hand] = 113;
                special = 1;
            }
        }
    }
    if (func_8008230C(total) == 2)
    {
        D_800A8768[hand] = 113;
        g_bj_natural[hand] = 2;
        CasinoQueueCluts(899, 12, 0x7CA4);
        CasinoStartAnim(D_8009C42C + hand * 16, 32, 0, 2048, 0, 0, 0, 0, 0, 0, 0);
    }
    if (D_800A8818[hand] == 1 && g_bj_natural[hand] == 1)
        D_800A8768[hand] = 113;
    if (g_bj_natural[hand] == 3 || g_bj_natural[hand] == 9 || special)
        func_8007FBD8(D_800A8674);
}
