/* Persona 1 (JP) - select the result palette and start its animation. */
#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <persona/casino/casino.h>
extern char D_80099A98[];
extern u_char palette_rank __asm__("D_800A85F8+0x27");
extern void CasinoQueueCluts();
extern void CasinoStartPalAnim(CasinoPalAnim *);

void func_80084444(u_char active)
{
    int offset = -(active == 0) & 0x60;
    u_char rank = palette_rank;
    CasinoPalAnim *animation;
    CasinoQueueCluts((s8)rank * 8 + 160, 1,
                    GetClut(*(short *)(D_80099A98 + offset + 8),
                            *(short *)(D_80099A98 + offset + 10)));
    animation = (CasinoPalAnim *)D_80099A98;
    if (active == 0) animation += 4;
    CasinoStartPalAnim(animation);
}
