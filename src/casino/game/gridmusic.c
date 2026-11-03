#include <persona/casino/casino.h>
#include <libsnd.h>

extern u_char D_800A8704[], D_800A8700[];
extern void CasinoPlaySeq(short *, u_long *, short);
extern void func_8008A9DC(int, short, short);
extern void CasinoSpritesSetOn(int, int, int);
extern void func_8008CE0C(void), func_8008D04C(void);

void func_8008945C(void)
{
    if (g_casino_timer == 0)
    {
        D_800A8704[1] = 1;
        SsSeqSetVol(g_casino_seqs[15], 0, 0);
        switch (D_800A8700[1])
        {
        case 0:
            CasinoPlaySeq(&g_casino_seqs[1], (u_long *)0x12F04C, g_casino_main_vab);
            break;
        case 1: case 2:
            CasinoPlaySeq(&g_casino_seqs[1], (u_long *)0x12F29C, g_casino_main_vab);
            break;
        case 3: case 4:
            CasinoPlaySeq(&g_casino_seqs[1], (u_long *)0x12F3CC, g_casino_main_vab);
            break;
        case 5: case 6: case 7: case 8: case 9:
            CasinoPlaySeq(&g_casino_seqs[1], (u_long *)0x12F4AC, g_casino_main_vab);
            break;
        }
        func_8008A9DC(2, -128, 32);
    }
    if (!(g_casino_timer & 3))
        CasinoSpritesSetOn(16, 1, (g_casino_timer / 4) % 2);
    if (!SsIsEos(g_casino_seqs[1], 0))
    {
        func_8008CE0C();
        func_8008D04C();
        CasinoSpritesSetOn(16, 1, 1);
        g_casino_step = 115;
        g_casino_timer = -1;
    }
}
