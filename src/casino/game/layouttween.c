/* Persona 1 (JP) - choose a layout texture and start its horizontal tween. */
#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <persona/casino/casino.h>

extern CasinoLayout *D_800A2BC8[];
extern void CasinoShowLayout(CasinoLayout *, u_char);
extern void CasinoTween(CasinoLayout *, int, short, short, short, short);

void func_8008A9DC(int texture, short distance, short frames)
{
    *D_800A2BC8[217]->tex_idx = texture & 255;
    CasinoShowLayout(D_800A2BC8[217], 0);
    CasinoTween(D_800A2BC8[217], distance, 0, 0, 0, frames);
}
