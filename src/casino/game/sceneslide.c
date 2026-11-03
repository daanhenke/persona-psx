/* Persona 1 (JP) - slide a casino layout, then advance its state. */
#include <decomp/types.h>
extern void *slide_layouts[][2] __asm__("D_800A2BC8+0x46C");
extern void *scroll_layout __asm__("D_800A2BC8+0x4BC");
extern u_char slide_side __asm__("D_800A8700+0x1");
extern int g_casino_timer;
extern s8 g_casino_step;
extern void CasinoTween();
extern void CasinoScroll();

void func_80089B68(void)
{
    if (g_casino_timer == 0) {
        CasinoTween(slide_layouts[(slide_side & 1) == 0][0], -160, 0, 0, 0, 32);
        CasinoScroll(scroll_layout, 0, 16, 0, 0, 16);
    }
    g_casino_step = 30;
    g_casino_timer = -1;
}
