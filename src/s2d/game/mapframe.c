/* Map frame submission, clipping, and framebuffer transfer. */
typedef struct
{
    int a, b, c, d;
    void *ot;
} Ot;
extern Ot g_ot_obj[], g_ot_map[], g_ot_back[];
extern char g_scene_drmode[][12];
extern int g_draw_side, g_s2d_frame, D_800B0EF0;
extern short D_800B8FE4[2], D_800B8FEC[2], D_800B8FF4[2], D_800B8FFC[2], D_800B9428[2],
    D_800B942C[2];
extern int D_800B91C0;
extern unsigned short D_800B91EC[];
extern short g_s2d_drawenv[];
extern void S2dBeginFrame(int), AddPrim(void *, void *), GsSortPoly(void *, Ot *, int),
    S2dWinSetArea(void *, int, int, int), func_8009A368(void *, Ot *, int, int),
    S2dIndicatorStep(void), func_8009B758(Ot *, int), func_8009C36C(void), func_8009C478(Ot *),
    GetDrawEnv(void *), DrawSync(int), VramFlushQueues(void), ResetGraph(int), func_8008E1C0(void),
    GsSwapDispBuff(void), GsSortClear(int, int, int, Ot *), GsDrawOt(Ot *),
    MoveImage(void *, int, int);
extern int VSync(int);
void func_80093AE0(void)
{
    S2dBeginFrame(1);
    AddPrim(g_ot_obj[g_draw_side].ot, &g_scene_drmode[g_draw_side]);
    D_800B8FE4[0] = -256;
    D_800B8FEC[0] = 256;
    D_800B8FF4[0] = -256;
    D_800B8FFC[0] = 256;
    D_800B8FE4[1] = 128 - D_800B91C0 * 2;
    D_800B8FEC[1] = 128 - D_800B91C0 * 2;
    D_800B8FF4[1] = (short)(128 + D_800B91C0) - D_800B91C0 * 2;
    D_800B8FFC[1] = (short)(128 + D_800B91C0) - D_800B91C0 * 2;
    GsSortPoly((char *)D_800B8FE4 - 8, &g_ot_map[g_draw_side], 2047);
    if (D_800B91EC[1])
    {
        S2dWinSetArea(D_800B91EC, g_draw_side, g_s2d_drawenv[4], g_s2d_drawenv[5]);
        func_8009A368(D_800B91EC, &g_ot_back[g_draw_side], 0, g_draw_side);
    }
    S2dIndicatorStep();
    func_8009B758(&g_ot_map[g_draw_side], 0);
    func_8009C36C();
    func_8009C478(&g_ot_map[g_draw_side]);
    GetDrawEnv(g_s2d_drawenv);
    DrawSync(0);
    VramFlushQueues();
    D_800B0EF0 = VSync(0);
    ResetGraph(1);
    func_8008E1C0();
    GsSwapDispBuff();
    GsSortClear(0, 0, 0, &g_ot_obj[g_draw_side]);
    GsDrawOt(&g_ot_obj[g_draw_side]);
    D_800B9428[0] = 0;
    D_800B9428[1] = 256 + D_800B91C0;
    D_800B942C[0] = 320;
    D_800B942C[1] = 240 - D_800B91C0;
    MoveImage(D_800B9428, (1 - g_draw_side) * 512, 0);
    GsDrawOt(&g_ot_map[g_draw_side]);
    GsDrawOt(&g_ot_back[g_draw_side]);
    g_s2d_frame++;
}
