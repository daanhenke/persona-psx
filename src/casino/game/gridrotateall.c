#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <persona/casino/casino.h>

extern CasinoModel vertical_models[] __asm__("D_800A2BC8+0x144C");
extern CasinoModel horizontal_models[] __asm__("D_800A2BC8+0x16C8");
extern void        CasinoTexObj(CasinoModel*), CasinoStartAnim();
extern void        SsSeqStop(short), SsSeqPlay(short, short, short);
void               func_8008B10C(signed char direction, short speed)
{
    short         vtex = 0, htex = 0, vface = 0, hface = 0, angle = 0;
    int           row, col;
    unsigned char index;
    if (direction == 1)
    {
        vtex  = 0;
        htex  = 1;
        vface = 1;
        hface = 1;
        angle = -2048;
    }
    else if (direction == -1)
    {
        vtex  = 4;
        htex  = 8;
        vface = 0;
        hface = 0;
        angle = 2048;
    }
    SsSeqStop(g_casino_seqs[10]);
    SsSeqPlay(g_casino_seqs[10], 1, 1);
    for (row = 0; row < 4; row++)
    {
        for (col = 0; col < 3; col++)
        {
            if (row / 2 == 0)
            {
                index                                 = row * 3 + col;
                vertical_models[index].tex_idx[vface] = vtex;
                CasinoTexObj(&vertical_models[index]);
                CasinoStartAnim(vertical_models[index].obj, 2048 / speed, 0, angle, 0, 0, 0, 0, 0, 0, 0);
            }
            else
            {
                index                                   = (row - 2) * 3 + col;
                horizontal_models[index].tex_idx[hface] = htex;
                CasinoTexObj(&horizontal_models[index]);
                CasinoStartAnim(horizontal_models[index].obj, 2048 / speed, angle, 0, 0, 0, 0, 0, 0, 0, 0);
            }
        }
    }
}
