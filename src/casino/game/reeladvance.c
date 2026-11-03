/* Persona 1 (JP) - advance a reel after one full symbol-height step. */
#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <persona/casino/casino.h>

extern CasinoLayout* reel_layouts[2][21] __asm__("D_800AF834+0x8");
extern s8            reel_travel[21] __asm__("D_800AF834+0x12E");
extern s8            reel_position[21] __asm__("D_800AF834+0x158");
extern u_char*       D_800B3E28[];
extern void          CasinoTween();
extern void          CasinoScroll();

void func_800868A0(u_char n)
{
    u_int k = (unsigned char)n;
    short i;
    s8    position;
    short delta = 16;
    if (k < 5)
        delta = 32;
    i = k;
    if (reel_travel[i] == delta)
    { /* Reuse this position for the first texture; the second is reread. */
        position = reel_position[i];
        if (position == 27)
        {
            reel_layouts[0][i]->tex_idx[0] = D_800B3E28[i][0] + 8;
            reel_layouts[1][i]->tex_idx[0] = D_800B3E28[i][27];
            reel_position[i]               = -1;
        }
        else
        {
            reel_layouts[0][k]->tex_idx[0] = D_800B3E28[k][position + 1] + 8;
            reel_layouts[1][k]->tex_idx[0] = D_800B3E28[k][reel_position[k]];
        }
        CasinoTween(reel_layouts[0][(unsigned char)n], 0, 0, 0, -delta, 1);
        CasinoScroll(reel_layouts[0][(unsigned char)n], 0, delta, 0, -delta, 1);
        CasinoTween(reel_layouts[1][(unsigned char)n], 0, -delta, 0, delta, 1);
        CasinoScroll(reel_layouts[1][(unsigned char)n], 0, 0, 0, delta, 1);
        reel_travel[(unsigned char)n] = 0;
        reel_position[(unsigned char)n]++;
    }
}
