#include <persona/casino/casino.h>
#include <persona/casino/grid.h>
#include <libsnd.h>
extern void CasinoBuildLayouts(CasinoLayoutDef *, int);
extern CasinoSymbolBlock D_800655B0;
extern CasinoDigitRows   D_800655BC, D_800655C8;
extern unsigned char     D_800A8700[4], D_800A8704[4];
extern unsigned char     grid_player __asm__("D_800A8700+1");
extern short             grid_ticks __asm__("D_800AF9E8+0xD0");
extern short             D_800A86FC;
extern signed char       D_800AFC00[3][3];
extern CasinoLayout*     scroll_layout __asm__("D_800A2BC8+0x4BC");
extern CasinoLayoutDef   round_defs[] __asm__("D_800A2BC8+0xDC");
extern CasinoLayoutDef   choices[] __asm__("D_800A2BC8+0x468");
extern unsigned char*    cell_objects[9][5] __asm__("D_800A2BC8+0x11E0");
extern CasinoObj*        all_objects[] __asm__("D_800A2BC8");
extern void              CasinoScroll(), CasinoTween(CasinoLayout*, int, short, short, short, short);
extern void              CasinoStartAnim(CasinoObj*, short, short, short, short, long, long, long, long, long, long);
extern void              func_8008AA64(int, int), func_8008AD00(unsigned char[4][3], short), func_8008B10C(int, int), func_8008B428(void);
extern void              func_8008B870(), func_8008B93C(short);
void                     func_8008A010(void)
{
    int i, count = 0;
    int x, y;
    if (g_casino_timer == 0)
    {
        unsigned char* player = &grid_player;
        grid_ticks            = 0;
        if (*player >= 2)
            CasinoScroll(scroll_layout, 0, -((*player - 1) * 16), 0, 0, 1);
        D_800A86FC = 0;
        i          = 0;
        func_8008B428();
        CasinoBuildLayouts(round_defs, 1);
        if (i < *player)
            do
            {
                CasinoSymbolBlock symbols = D_800655B0;
                CasinoDigitRows   digits  = D_800655BC;
                func_8008AAF8((unsigned char)i, symbols);
                func_8008ABD8((unsigned char)i, digits);
                func_8008AA64((unsigned char)i, 0);
                i++;
            } while (i < grid_player);
        if (D_800A8704[1])
            CasinoTween(choices[!(grid_player & 1)].l, -160, 0, 0, 0, 32);
    }
    if (g_casino_timer >= 32)
        CasinoBuildLayouts(choices, 0);
    if (g_casino_timer >= 0)
    {
        for (y = 0; y < 3; y++)
            for (x = 0; x < 3; x++)
            {
                if (D_800AFC00[y][x])
                {
                    if (*cell_objects[y * 3 + x][0] == 0)
                    {
                        SsSeqStop(g_casino_seqs[9]);
                        SsSeqPlay(g_casino_seqs[9], 1, 1);
                        g_casino_cursor.x = x;
                        g_casino_cursor.y = y;
                        if (D_800AFC00[y][x] < 5)
                        {
                            D_800AFC00[y][x] = (D_800AFC00[y][x] - 1) % 9;
                            if (D_800AFC00[y][x] < 0)
                                D_800AFC00[y][x] = 8;
                            func_8008B93C(128);
                        }
                        else
                        {
                            int next = D_800AFC00[y][x];
                            next++;
                            next            %= 9;
                            D_800AFC00[y][x] = next;
                            func_8008B870(128, next);
                        }
                    }
                }
                else
                    count++;
            }
    }
    if (D_800A8704[0] == 0)
    {
        if (grid_ticks == 0)
        {
            CasinoDigitRows digits = D_800655C8;
            func_8008AD00(digits.digit, 128);
        }
        grid_ticks--;
    }
    if (D_800A8704[0] == 1)
    {
        func_8008B10C(-1, 128);
        D_800A8704[0] = 2;
    }
    if (count == 9 && D_800A8704[0] == 2)
    {
        g_casino_step  = 0x12;
        D_800A8704[1]  = 0;
        g_casino_timer = -16;
        {
            u_int* lamps = &g_casino_lamps;
            *lamps      |= 5;
        }
        CasinoStartAnim(all_objects[817], 16, 0, 0, 0, 0, 16, 0, 0, 0, 0);
        CasinoStartAnim(all_objects[851], 16, 0, 0, 0, 0, 16, 0, 0, 0, 0);
    }
}