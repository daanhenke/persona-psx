#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <persona/casino/casino.h>

/* The current index is independently addressed by the original code. */
extern unsigned char grid_current __asm__("D_800A8700+1");
extern unsigned char D_800A8700[], D_800A8704[];
extern unsigned short D_800AF9E8[];
extern CasinoLayout *D_800A2BC8[];
extern void CasinoPlaySeq(short *, unsigned long *, short);
extern void func_8008C2DC(int), CasinoShowLayout(CasinoLayout *, unsigned char);

void func_800899F8(void)
{
    unsigned short tick;

    if (g_casino_timer == 0)
    {
        D_800AF9E8[104] = 0;
        CasinoPlaySeq(g_casino_seqs, (unsigned long *)0x12E818, g_casino_vab);
    }
    if (grid_current == D_800A8700[0] - 1)
    {
        tick = D_800AF9E8[104];
        if (!(tick & 31) && tick != 0)
            CasinoPlaySeq(g_casino_seqs, (unsigned long *)0x12E818, g_casino_vab);
        func_8008C2DC(192);
    }
    else
    {
        func_8008C2DC(64);
        if (g_casino_pad_trig & 32 || g_casino_pad_trig & 128 ||
            g_casino_pad_trig & 16 || g_casino_pad_trig & 64 ||
            g_casino_pad_trig & 8 || g_casino_pad_trig & 2)
        {
            CasinoShowLayout(D_800A2BC8[217], 1);
            D_800A8704[0] = 1;
        }
    }
    if (D_800A8704[0] == 1)
    {
        D_800A8704[0] = 0;
        if (grid_current++ == D_800A8700[0] - 1)
            g_casino_step = 118;
        else
            g_casino_step = 117;
        g_casino_timer = -1;
    }
}
