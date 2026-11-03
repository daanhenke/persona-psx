#include <persona/s2d/s2d.h>

void func_8008AC10(void)
{
    int i;
    ResetGraph(1);
    AddPrim(g_ot_back[g_draw_side].tag, &g_menu_drmode[g_draw_side]);
    AddPrim(g_ot_obj[g_draw_side].tag, &g_scene_drmode[g_draw_side]);
    GetDrawEnv(&g_s2d_drawenv);
    GsSortClear(g_fade.rgb[0], g_fade.rgb[1], g_fade.rgb[2], &g_ot_obj[g_draw_side]);
    GsDrawOt(&g_ot_obj[g_draw_side]);
    GsDrawOt(&g_ot_map[g_draw_side]);
    GsDrawOt(&g_ot_front[g_draw_side]);
    GsDrawOt(&g_ot_back[g_draw_side]);
    for (i = 0; i < 3; i++)
    {
        GsDrawOt(&g_ot_layer[i][g_draw_side]);
    }
    DrawSync(0);
    g_s2d_frame++;
}
