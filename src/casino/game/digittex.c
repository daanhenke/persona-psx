/* Persona 1 (JP) - set the two decimal digit textures for a layout. */
#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <persona/casino/casino.h>
extern CasinoLayout *digit_layouts[][2] __asm__("D_800A2BC8+0x2A0C");
extern void CasinoTexLayout(CasinoLayout *);

void func_80091154(u_char index, u_char value)
{
    digit_layouts[index][0]->tex_idx[0] = (u_char)(value / 10);
    digit_layouts[index][0]->tex_idx[1] = (u_char)(value % 10);
    CasinoTexLayout(digit_layouts[index][0]);
}
