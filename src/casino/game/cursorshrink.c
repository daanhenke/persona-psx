/* Persona 1 (JP) - animate the selected cursor object. */
#include <decomp/types.h>
extern u_char g_casino_cursor[];
extern void *cursor_objects[][5] __asm__("D_800A2BC8+0x11E0");
extern void func_8008BA08(void);
extern void CasinoStartAnim();

void func_8008B93C(short duration)
{
    u_char x = g_casino_cursor[0];
    u_char y = g_casino_cursor[1];
    func_8008BA08();
    CasinoStartAnim(cursor_objects[x + y * 3][0], 0x800 / duration, -0x800, 0, 0, 0, 0, 0, 0, 0, 0);
}
