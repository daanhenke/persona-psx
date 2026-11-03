#include <persona/s2d/s2d.h>

extern short g_map_id;
extern DR_MODE D_800B85F0, D_800B85FC;

void func_80096904(void)
{
    switch (g_map_info[g_map_id].effect)
    {
    case 0:
        SetDrawMode(&D_800B85F0, 0, 1, 0, 0);
        D_800B85FC = D_800B85F0;
        SetDrawMode(&g_scene_drmode[0], 0, 1, 32, 0);
        g_scene_drmode[1] = g_scene_drmode[0];
        break;
    case 1:
        SetDrawMode(&D_800B85F0, 0, 1, 64, 0);
        D_800B85FC = D_800B85F0;
        SetDrawMode(&g_scene_drmode[0], 0, 1, 64, 0);
        g_scene_drmode[1] = g_scene_drmode[0];
        break;
    }
}
