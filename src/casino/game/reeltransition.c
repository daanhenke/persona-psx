/* Persona 1 (JP) - pair reel layout tweens with texture scrolling. */
#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <persona/casino/casino.h>
extern CasinoLayout *reel_layouts[2][21] __asm__("D_800AF834+0x8");
extern u8 reel_frames[] __asm__("D_800AF834+0x104");
extern s8 reel_delta[] __asm__("D_800AF834+0x119");
extern void CasinoTween();
extern void CasinoScroll();
void func_800867A8(u_char index)
{
    u_char frames = reel_frames[index];
    s8 delta = reel_delta[index];
    CasinoTween(reel_layouts[0][index], 0, 0, 0, delta, (s8)frames);
    CasinoScroll(reel_layouts[0][index], 0, -delta, 0, delta, (s8)frames);
    CasinoTween(reel_layouts[1][index], 0, delta, 0, -delta, (s8)frames);
    CasinoScroll(reel_layouts[1][index], 0, 0, 0, -delta, (s8)frames);
}
