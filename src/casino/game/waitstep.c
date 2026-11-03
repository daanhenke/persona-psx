/* Persona 1 (JP) - advance after the pending operation finishes. */
#include <decomp/types.h>

extern u_char func_8008EAA4(void);
extern int g_casino_timer;
extern u_char g_casino_step;

void func_8008D6A4(void)
{
    if (!func_8008EAA4()) {
        g_casino_timer = -1;
        g_casino_step = 0x36;
    }
}
