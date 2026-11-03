/* Check the current tile against the two blinking barrier states. */
#include <decomp/types.h>
extern short D_800B8FD4, D_800B8FD8;
extern u_char D_800B1D30[];

#ifndef PERSONA_TILE_MAP
#define PERSONA_TILE_MAP ((u_char *)0x80122000)
#endif

int func_8008CA28(int flags)
{
    int allow = 1;
    u_short tile;
    u_char *map;
    if (flags & 0x80) return 1;
    map = PERSONA_TILE_MAP;
    tile = *(u_short *)(map + (D_800B8FD4 * 2 +
             ((199 - D_800B8FD8) * 400 + *(int *)map)));
    if (tile & 0x2000)
        if (D_800B1D30[0] == 0)
            if (tile & 0x1000) allow = 0;
    map = PERSONA_TILE_MAP;
    tile = *(u_short *)(map + (D_800B8FD4 * 2 +
             ((199 - D_800B8FD8) * 400 + *(int *)map)));
    if (tile & 0x4000)
        if (D_800B1D30[1] == 0)
            if (tile & 0x1000) allow = 0;
    return allow;
}
