#include <persona/s2d/s2d.h>
#include <libetc.h>
extern void S2dBeginFrame(int);
extern short D_8005E5A4, D_8005E5A6, D_8005E5A8, D_8005E5AA, D_8005E600, D_8005E602;

/* Keep the rectangle, transform and strip arrays in one workspace. The
   byte-offset view preserves the original compiler's address calculation. */
#define CAPTURE_POLY(i)                                                                            \
    ((POLY_FT4 *)((i) * sizeof(POLY_FT4) + (u_char *)&capture + sizeof(capture) -                  \
                  sizeof(capture.q)))

/* Project the captured field into four strips and copy the result to VRAM. */
void func_80095E34(void) {
    struct {
        RECT rect;
        SVECTOR rot;
        VECTOR trans, scale;
        MATRIX m;
        SVECTOR v[4];
        POLY_FT4 q[4];
    } capture;
    SVECTOR *vptr;
    long ox, oy, j;
    int i;
    POLY_FT4 *poly;
    POLY_FT4 *polys = capture.q;
    SVECTOR *vertices = capture.v;
    for (i = 0; i < 4; i++) {
        poly = &polys[i];
        SetPolyFT4(poly);
        SetSemiTrans(poly, 0);
        SetShadeTex(poly, 0);
        CAPTURE_POLY(i)->u0 = 0;
        CAPTURE_POLY(i)->v0 = 0;
        CAPTURE_POLY(i)->u1 = 128;
        CAPTURE_POLY(i)->v1 = 0;
        CAPTURE_POLY(i)->u2 = 0;
        CAPTURE_POLY(i)->v2 = 240;
        CAPTURE_POLY(i)->u3 = 128;
        CAPTURE_POLY(i)->v3 = 240;
        CAPTURE_POLY(i)->r0 = 128;
        CAPTURE_POLY(i)->g0 = 128;
        CAPTURE_POLY(i)->b0 = 128;
        poly->tpage = GetTPage(2, 0, i * 128, 256);
    }
    ReadGeomOffset(&ox, &oy);
    SetGeomOffset(-256, -120);
    capture.trans.vx = 0;
    capture.trans.vy = 0;
    capture.trans.vz = 0;
    capture.rot.vx = 0;
    capture.rot.vy = 0;
    capture.rot.vz = 0;
    capture.scale.vx = 2564;
    capture.scale.vy = 4096;
    capture.scale.vz = 4096;
    RotMatrix(&capture.rot, &capture.m);
    TransMatrix(&capture.m, &capture.trans);
    ScaleMatrix(&capture.m, &capture.scale);
    SetRotMatrix(&capture.m);
    SetTransMatrix(&capture.m);
    for (i = 0; i < 4; i++) {
        vertices = capture.v;
        for (j = 0; j < 4; j++) {
            vptr = vertices + j;
            vptr->vx = (i + (j & 1)) * 128 - 4;
            vptr->vy = (j / 2) * 240;
            vptr->vz = 640;
        }
        polys = capture.q;
        RotTransPers4(&capture.v[0], &capture.v[1], &capture.v[2], &capture.v[3],
                      (long *)&polys[i].x0, (long *)&polys[i].x1, (long *)&polys[i].x2,
                      (long *)&polys[i].x3, &j, &j);
    }
    SetGeomOffset(ox, oy);
    S2dBeginFrame(1);
    for (i = 0; i < 4; i++)
        GsSortPoly(&polys[i], &g_ot_obj[g_draw_side], 63);
    GsSortClear(0, 0, 0, &g_ot_obj[g_draw_side]);
    GsDrawOt(&g_ot_obj[g_draw_side]);
    DrawSync(0);
    capture.rect = g_s2d_drawenv.clip;
    MoveImage(&capture.rect, 0, 256);
    DrawSync(0);
    VSync(0);
    D_8005E5A4 = 320;
    D_8005E5A6 = 240;
    D_8005E5A8 = 0;
    D_8005E5AA = 120;
    D_8005E600 = 320;
    D_8005E602 = 240;
}

#undef CAPTURE_POLY
