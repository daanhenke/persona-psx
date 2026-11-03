/* Persona 1 (JP) - query the map cell's 0x400 flag. */
#include <decomp/types.h>

extern s16 D_800B8FD4;
extern s16 D_800B8FD8;

bool func_8008BB84(void)
{
    u_char *map = (u_char *)0x80122000;
    bool result = false;
    if ((*(s16 *)(map + (D_800B8FD4 * 2 +
                      (D_800B8FD8 * 400 + *(int *)map))) & 0x400) != 0) {
        result = true;
    }
    return result;
}
