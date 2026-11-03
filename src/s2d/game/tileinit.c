/* Clear the 200-by-200 map's transient flags and initialize map-zero barriers. */
#include <decomp/types.h>

extern u_char D_800A5168[][2], D_800A519C[][2];
extern u_char D_800A51F0[][2], D_800A5314[][2];
extern u_char *packed_map __asm__("D_800B1D30+4");
extern u_char D_800B0FAC[];
extern u_short tile_map_id __asm__("g_map_id");
extern void *bzero(u_char *, int);
extern void func_80094660(void);

#ifndef PERSONA_TILE_MAP
#define PERSONA_TILE_MAP ((u_char *)0x80122000)
#endif
#define TILE_OFFSET(o, x, y) \
    (*(u_short *)(PERSONA_TILE_MAP + (o) + (y) * 400 + (x) * 2))
#define TILE(x, y) TILE_OFFSET(*(u_int *)PERSONA_TILE_MAP, x, y)

void func_80094B08(void)
{
    int i;
    for (i = 0; i <= 39999; i++)
        TILE(i % 200, i / 200) &= 0x87FF;
    bzero(packed_map, 20000);
    bzero(D_800B0FAC, 3456);
    if (tile_map_id == 0)
    {
        for (i = 0; D_800A51F0[i][0] != 255; i++)
        {
            u_int offset = *(u_int *)PERSONA_TILE_MAP;
            TILE_OFFSET(offset, D_800A51F0[i][0], D_800A51F0[i][1]) |= 0x2000;
            TILE_OFFSET(offset, D_800A51F0[i][0], D_800A51F0[i][1]) |= 0x1000;
        }
        for (i = 0; D_800A5314[i][0] != 255; i++)
        {
            u_int offset = *(u_int *)PERSONA_TILE_MAP;
            TILE_OFFSET(offset, D_800A5314[i][0], D_800A5314[i][1]) |= 0x4000;
            TILE_OFFSET(offset, D_800A5314[i][0], D_800A5314[i][1]) |= 0x1000;
        }
        for (i = 0; D_800A5168[i][0] != 255; i++)
            TILE(D_800A5168[i][0], D_800A5168[i][1]) |= 0x2000;
        for (i = 0; D_800A519C[i][0] != 255; i++)
            TILE(D_800A519C[i][0], D_800A519C[i][1]) |= 0x4000;
    }
    func_80094660();
}
