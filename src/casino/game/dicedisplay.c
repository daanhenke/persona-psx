extern unsigned char g_casino_cursor[], g_casino_bet_done, g_casino_quit, g_casino_step,
    D_800AAE94[], D_800AAE9C[], D_800AAEA8;
extern int g_casino_money, g_casino_money_shown, g_casino_win, D_800A8EA4;
extern unsigned char *D_800A2BC8[];
extern char D_800B0720[];
typedef struct
{
    short a[8];
    unsigned char active;
    char pad[7];
} Pal;
extern unsigned char paletteflag __asm__("D_800999C0+16");
extern Pal D_800999C0, D_80099CC0, D_80099CD8, D_80099CF0, D_80099D08;
extern unsigned short GetClut(int, int);
extern void func_800905CC(void), func_800906D0(void), func_80090EEC(int),
    CasinoShowNumber(int, void *, int), func_80090828(void), func_80090A50(void),
    func_80090C58(void), func_8008FEC4(void), CasinoStartPalAnim(void *), CasinoStartAnim(),
    func_80092F14(void *, void *), CasinoTween(void *, short, short, short, short, short),
    CasinoQueueCluts(int, int, unsigned short);
void func_8008EE00(void)
{
    func_800905CC();
    func_800906D0();
    func_80090EEC(g_casino_cursor[1]);
    if (g_casino_money != g_casino_money_shown)
    {
        CasinoShowNumber(g_casino_money, D_800A2BC8[0x20f0 / 4], 8);
        g_casino_money_shown = g_casino_money;
    }
    if (g_casino_win != D_800A8EA4)
    {
        CasinoShowNumber(g_casino_win, D_800A2BC8[0x1efc / 4], 8);
        D_800A8EA4 = g_casino_win;
    }
    if (g_casino_bet_done == 1)
    {
        if (D_800AAE94[0])
            func_80090828();
        else
            g_casino_bet_done = 0;
    }
    if (g_casino_quit == 1)
    {
        if (D_800AAE94[1])
            func_80090A50();
        else
            g_casino_quit = 0;
    }
    func_80090C58();
    if (g_casino_step == 17 || g_casino_step == 18 || g_casino_step == 38)
        func_8008FEC4();
    if (g_casino_step == 17 || g_casino_step == 18 || g_casino_step == 38 || g_casino_step == 36)
        if (!paletteflag)
            CasinoStartPalAnim(&D_800999C0);
    if (g_casino_step == 17 || g_casino_step == 18 || g_casino_step == 38)
        if (!D_800A2BC8[0x31e0 / 4][0])
            CasinoStartAnim(D_800A2BC8[0x31e0 / 4], 1, 66, 0, 0, 0, 0, 0, 0, 0, 0);
    func_80092F14(&D_800A2BC8[0x39e0 / 4], D_800B0720);
    func_80092F14(&D_800A2BC8[0x39f4 / 4], D_800B0720 + 240);
    if (D_800AAE9C[9] != D_800AAE9C[10])
    {
        if (D_800AAE9C[9])
        {
            D_800AAE9C[11] = 0;
            CasinoTween(D_800A2BC8[0x1d38 / 4], 0, -13, 0, 26, 13);
            CasinoTween(D_800A2BC8[0x1d40 / 4], 0, 13, 0, -26, 13);
            CasinoQueueCluts(32, 1, GetClut(D_80099CC0.a[4], D_80099CC0.a[5]));
            if (!D_80099CC0.active)
                CasinoStartPalAnim(&D_80099CC0);
        }
        else
        {
            CasinoTween(D_800A2BC8[0x1d38 / 4], 0, 13, 0, -26, 13);
            CasinoTween(D_800A2BC8[0x1d40 / 4], 0, -13, 0, 26, 13);
            CasinoQueueCluts(32, 1, GetClut(D_80099CD8.a[4], D_80099CD8.a[5]));
            if (!D_80099CD8.active)
                CasinoStartPalAnim(&D_80099CD8);
        }
        D_800AAE9C[10] = D_800AAE9C[9];
    }
    if (D_800AAE9C[11] != D_800AAEA8)
    {
        if (D_800AAE9C[11])
        {
            CasinoQueueCluts(33, 1, GetClut(D_80099CF0.a[4], D_80099CF0.a[5]));
            if (!D_80099CF0.active)
                CasinoStartPalAnim(&D_80099CF0);
        }
        else
        {
            CasinoQueueCluts(33, 1, GetClut(D_80099D08.a[4], D_80099D08.a[5]));
            if (!D_80099D08.active)
                CasinoStartPalAnim(&D_80099D08);
        }
        D_800AAEA8 = D_800AAE9C[11];
    }
    CasinoStartAnim(D_800A2BC8[0x3b7c / 4], 16, 32, 16, 0, 0, 0, 0, 0, 0, 0);
}
