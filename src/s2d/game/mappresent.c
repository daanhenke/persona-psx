#include <persona/s2d/s2d.h>
#include <libetc.h>
/* The width/height tail of D_800B9428 is followed by four captured strips
   and the bottom gradient. The rectangle and tail retain their original
   overlapping linker labels. */
typedef struct {
    short w, h;
    POLY_FT4 poly[4];
    POLY_G4 shade;
} CaptureTail;
extern CaptureTail D_800B942C;
extern RECT D_800B9428;
extern POLY_G4 D_800B8FDC;
extern u_char D_800B94D4[3], D_800B94DC[3], D_800B94E4[3], D_800B94EC[3];
extern u_short D_800B8FD4, D_800B8FD8, D_800B91E0, D_800B91E4;
extern int g_view_zoom, D_800B91C0, D_800A4CEC, D_800B0F18;
extern short D_8005E5A4, D_8005E5A6, D_8005E5A8, D_8005E5AA, D_8005E600, D_8005E602;
extern void S2dMsgClose(void), S2dBeginFrame(int), S2dDrawParty(GsOT *, int), S2dDrawMapObjs(void);
extern void func_800971D4(void), func_8008AC10(void), func_80099930(int), func_80095E34(void);
extern void S2dLoadWinColors(void), func_80093AE0(void), func_80089F5C(int), S2dMsgOpen(int),
    func_80096904(void);
extern int func_8009C07C(void);
extern void VramStore(int, int, int, int, u_long *), VramLoad(int, int, int, int, u_long *),
    VramMove(int, int, int, int, int, int);

/* Present the map window, wait for its response, and restore field rendering. */
void func_80093DF8(void)
{
    MATRIX matrix;
    SVECTOR rotation;
    VECTOR scale;
    VECTOR translation;
    SVECTOR vertices[4];
    long old_x, old_y, k;
    int state = 0;
    int i;
    POLY_FT4 *poly;
    D_800B91EC.flags = 0;
    g_view_zoom = 0;
    D_800B8FD4 = D_800B91E0;
    D_800B8FD8 = D_800B91E4;
    S2dMsgClose();
    S2dBeginFrame(0);
    S2dDrawParty(&g_ot_map[g_draw_side], 3);
    S2dDrawMapObjs();
    func_800971D4();
    func_8008AC10();
    VramStore(0, 256, 192, 256, (u_long *)0x800E8000);
    VramStore(512, 256, 192, 248, (u_long *)0x800D0000);
    VramMove(320, 256, 192, 248, 512, 256);
    func_80099930(1);
    DrawSync(0);
    D_800B9428 = g_s2d_drawenv.clip;
    MoveImage(&D_800B9428, 0, 256);
    SetPolyG4(&D_800B8FDC);
    SetSemiTrans(&D_800B8FDC, 1);
    SetShadeTex(&D_800B8FDC, 0);
    D_800B8FDC.r0 = 0;
    D_800B8FDC.g0 = 0;
    D_800B8FDC.b0 = 0;
    D_800B8FDC.r1 = 0;
    D_800B8FDC.g1 = 0;
    D_800B8FDC.b1 = 0;
    D_800B8FDC.r2 = 255;
    D_800B8FDC.g2 = 255;
    D_800B8FDC.b2 = 255;
    D_800B8FDC.r3 = 255;
    D_800B8FDC.g3 = 255;
    D_800B8FDC.b3 = 255;
    SetPolyG4(&D_800B942C.shade);
    SetSemiTrans(&D_800B942C.shade, 1);
    SetShadeTex(&D_800B942C.shade, 0);
    D_800B94D4[0] = 255;
    D_800B94D4[1] = 255;
    D_800B94D4[2] = 255;
    D_800B94DC[0] = 255;
    D_800B94DC[1] = 255;
    D_800B94DC[2] = 255;
    D_800B94E4[0] = 0;
    D_800B94E4[1] = 0;
    D_800B94E4[2] = 0;
    D_800B94EC[0] = 0;
    D_800B94EC[1] = 0;
    D_800B94EC[2] = 0;
    func_80095E34();
    SetDrawMode(&g_scene_drmode[0], 0, 1, 64, 0);
    g_scene_drmode[1] = g_scene_drmode[0];
    D_800B91C0 = 0;
    S2dLoadWinColors();
    do {
        switch (state) {
        case 0:
            if (D_800B91C0 == 48)
                state = 1;
            else {
                D_800B91C0 += 2;
                if (D_800B91C0 >= 49)
                    D_800B91C0 = 48;
            }
            break;
        case 1:
            if (func_8009C07C() == state)
                state = 2;
            break;
        case 2:
            if (D_800B91C0 == 0)
                state = 3;
            else {
                D_800B91C0 -= 2;
                if (D_800B91C0 < 0)
                    D_800B91C0 = 0;
            }
            break;
        }
        func_80093AE0();
    } while (state != 3);
    for (i = 0; i < 4; i++) {
        poly = &D_800B942C.poly[i];
        SetPolyFT4(&D_800B942C.poly[i]);
        SetSemiTrans(&D_800B942C.poly[i], 0);
        SetShadeTex(&D_800B942C.poly[i], 0);
        poly->u0 = 0;
        poly->v0 = 0;
        poly->u1 = 128;
        poly->v1 = 0;
        poly->u2 = 0;
        poly->v2 = 240;
        poly->u3 = 128;
        poly->v3 = 240;
        poly->r0 = 128;
        poly->g0 = 128;
        poly->b0 = 128;
        D_800B942C.poly[i].tpage = GetTPage(2, 0, i << 7, 256);
    }
    ReadGeomOffset(&old_x, &old_y);
    SetGeomOffset(-256, -120);
    scale.vx = 0x1999;
    translation.vx = 0;
    translation.vy = 0;
    translation.vz = 0;
    rotation.vx = 0;
    rotation.vy = 0;
    rotation.vz = 0;
    scale.vy = 4096;
    scale.vz = 4096;
    RotMatrix(&rotation, &matrix);
    TransMatrix(&matrix, &translation);
    ScaleMatrix(&matrix, &scale);
    SetRotMatrix(&matrix);
    SetTransMatrix(&matrix);
    for (i = 0; i < 4; i++) {
        for (k = 0; k < 4; k++) {
            vertices[k].vx = (i + (int)(k & 1)) * 128 + 3;
            vertices[k].vy = (k / 2) * 240;
            vertices[k].vz = 640;
        }
        RotTransPers4(&vertices[0], &vertices[1], &vertices[2], &vertices[3],
                      (long *)&D_800B942C.poly[i].x0, (long *)&D_800B942C.poly[i].x1,
                      (long *)&D_800B942C.poly[i].x2, (long *)&D_800B942C.poly[i].x3, &k, &k);
    }
    SetGeomOffset(old_x, old_y);
    D_800B9428.x = 0;
    D_800B9428.y = 256;
    D_800B9428.w = 512;
    D_800B9428.h = 240;
    MoveImage(&D_800B9428, g_draw_side << 9, 0);
    DrawSync(0);
    S2dBeginFrame(1);
    for (i = 0; i < 4; i++)
        GsSortPoly(&D_800B942C.poly[i], &g_ot_obj[g_draw_side], 63);
    g_fade.rgb[0] = 0;
    g_fade.rgb[1] = 0;
    g_fade.rgb[2] = 0;
    func_80089F5C(2);
    DrawSync(0);
    VSync(0);
    D_8005E5A4 = 512;
    D_8005E5A6 = 240;
    D_8005E5A8 = 256;
    D_8005E5AA = 120;
    D_8005E600 = 512;
    D_8005E602 = 240;
    VramLoad(0, 256, 192, 256, (u_long *)0x800E8000);
    VramMove(512, 256, 128, 248, 320, 256);
    VramLoad(512, 256, 192, 248, (u_long *)0x800D0000);
    DrawSync(0);
    func_80099930(0);
    FntLoad(192, 256);
    D_800B0F18 = FntOpen(-240, -104, 512, 240, 0, 512);
    SetDumpFnt(D_800B0F18);
    if (D_800A4CEC != -1)
        S2dMsgOpen(D_800A4CEC);
    func_80096904();
    DrawSync(0);
}
