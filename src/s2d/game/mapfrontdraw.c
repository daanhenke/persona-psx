/* Persona 1 (JP) - dispatch the map's front-layer draw and advance its frame. */
#include <persona/s2d/s2d.h>
extern void func_80096E0C(GsOT *);
extern void func_80096A38(GsOT *);
extern int map_draw_frame __asm__("D_800A566C+0x10");
void func_800971D4(void)
{
    switch (g_btl_map_id) {
    case 0: case 1: case 2: break;
    case 4:
        func_80096E0C(&g_ot_front[g_draw_side]);
        break;
    case 3: case 5: case 6:
        func_80096A38(&g_ot_front[g_draw_side]);
        break;
    }
    map_draw_frame++;
}
