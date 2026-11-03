/* Persona 1 (JP) - query a neighboring map cell's 0x800 flag. */
#include <decomp/types.h>
extern void func_80094E68(void *, short, short *);

bool func_80094F2C(void *position, short facing)
{
    short cell[2];
    u_char *map;
    bool result = false;
    func_80094E68(position, facing, cell);
    map = (u_char *)0x80122000;
    if ((*(s16 *)(map + (cell[0] * 2 + (cell[1] * 400 + *(int *)map))) & 0x800) != 0) {
        result = true;
    }
    return result;
}
