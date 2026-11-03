/* Persona 1 (JP) - initialize the 11 by 11 wrapping cursor. */
#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <persona/casino/casino.h>

void func_80090E9C(void)
{
    g_casino_cursor.w = 11;
    g_casino_cursor.h = 11;
    g_casino_cursor.prev_x = 0;
    g_casino_cursor.x = 0;
    g_casino_cursor.prev_y = 0;
    g_casino_cursor.y = 0;
    g_casino_cursor.wrap_x = 1;
    g_casino_cursor.wrap_y = 1;
}
