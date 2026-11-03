#include <persona/s2d/s2d.h>
#include <libetc.h>

/* Captured screen strips, closing gradients and both frame buffers' lists. */
typedef struct
{
    POLY_FT4 poly[4];
    POLY_G4 shade[2];
    DR_MODE mode[2];
    GsOT ot[2];
    GsOT_TAG tags[2][4];
} MapCloseWorkspace;

extern MapCloseWorkspace* D_800B85BC;
extern int D_800A4D1C;
extern int D_800A4D20;
extern int g_draw_side;
extern DRAWENV g_s2d_drawenv;
extern void S2dDraw(void);
extern void S2dMsgClose(void);
extern void S2dLeaveToDng(void);

/* Close the captured field view toward its centre, then fade the seam white. */
void func_80091D24(void)
{
    RECT rect;
    short tpage;
    int activeBuffer;
    int i;
    int running;
    int intensity;
    POLY_G4* top;
    POLY_G4* bottom;
    MapCloseWorkspace* work;
    GsOT_TAG* tags;
    work = D_800B85BC;
    D_800A4D1C = 0;
    D_800A4D20 = 0;
    S2dDraw();
    DrawSync(0);
    S2dMsgClose();
    S2dLeaveToDng();
    rect = g_s2d_drawenv.clip;
    rect.x = 512 - rect.x;
    MoveImage(&rect, 0x80, 0x100);
    i = 0;

    do
    {
        /* Set the depth before deriving the tag pointer: this preserves the
           original compiler's ordering-table setup schedule. */
        work->ot[i].length = 2;
        tags = work->tags[i];

        work->ot[i].org = tags;
        i += 1;
    } while (i < 2);
    i = 0;

    do
    {
        SetPolyFT4(&work->poly[i]);
        SetSemiTrans(&work->poly[i], 0);
        SetShadeTex(&work->poly[i], 0);
        work->poly[i].y0 = -0x78;
        work->poly[i].r0 = 0x80;
        work->poly[i].g0 = 0x80;
        work->poly[i].b0 = 0x80;
        work->poly[i].x0 = (i * 128 - 256);
        work->poly[i].x1 = (i * 128 - 128);
        work->poly[i].y1 = -0x78;
        work->poly[i].x2 = (i * 128 - 256);
        work->poly[i].y2 = 0x78;
        work->poly[i].x3 = (i * 128 - 128);
        work->poly[i].y3 = 0x78;
        work->poly[i].u0 = 0;
        work->poly[i].v0 = 0;
        work->poly[i].u1 = 0x80;
        work->poly[i].v1 = 0;
        work->poly[i].u2 = 0;
        work->poly[i].v2 = 0xF0;
        work->poly[i].u3 = 0x80;
        work->poly[i].v3 = 0xF0;
        tpage = GetTPage(2, 0, i * 128 + 128, 256);
        work->poly[i].tpage = tpage;
        i += 1;

    } while (i < 4);
    top = &work->shade[0];
    SetPolyG4(top);
    SetSemiTrans(top, 1);
    SetShadeTex(top, 0);
    bottom = &work->shade[1];
    work->shade[0].r0 = 0xFF;
    work->shade[0].g0 = 0xFF;
    work->shade[0].b0 = 0xFF;
    work->shade[0].r1 = 0xFF;
    work->shade[0].g1 = 0xFF;
    work->shade[0].b1 = 0xFF;
    work->shade[0].r2 = 0;
    work->shade[0].g2 = 0;
    work->shade[0].b2 = 0;
    work->shade[0].r3 = 0;
    work->shade[0].g3 = 0;
    work->shade[0].b3 = 0;
    work->shade[0].x0 = -0x100;
    work->shade[0].y0 = -0x78;
    work->shade[0].x1 = 0x100;
    work->shade[0].y1 = -0x78;
    work->shade[0].x2 = -0x100;
    work->shade[0].y2 = -0x78;
    work->shade[0].x3 = 0x100;
    work->shade[0].y3 = -0x78;
    SetPolyG4(bottom);
    SetSemiTrans(bottom, 1);
    SetShadeTex(bottom, 0);
    work->shade[1].r0 = 0;
    work->shade[1].g0 = 0;
    work->shade[1].b0 = 0;
    work->shade[1].r1 = 0;
    work->shade[1].g1 = 0;
    work->shade[1].b1 = 0;
    work->shade[1].r2 = 0xFF;
    work->shade[1].g2 = 0xFF;
    work->shade[1].b2 = 0xFF;
    work->shade[1].r3 = 0xFF;
    work->shade[1].g3 = 0xFF;
    work->shade[1].b3 = 0xFF;
    work->shade[1].x0 = -0x100;
    work->shade[1].y0 = 0x78U;
    work->shade[1].x1 = 0x100;
    work->shade[1].y1 = 0x78U;
    work->shade[1].x2 = -0x100;
    work->shade[1].y2 = 0x78;
    work->shade[1].x3 = 0x100;
    work->shade[1].y3 = 0x78;
    SetDrawMode(&work->mode[0], 1, 1, 0x40, 0);
    SetDrawMode(&work->mode[1], 1, 1, 0x40, 0);
    /* This must reach 256; an eight-bit counter would never finish. */
    intensity = 0;
    running = 1;
    do
    {
        activeBuffer = GsGetActiveBuff();
        g_draw_side = activeBuffer;
        GsSetWorkBase((PACKET*)(((unsigned long)activeBuffer << 16) + 0x80100000));
        GsClearOt(0, 0, &work->ot[g_draw_side]);
        work->shade[0].y2 = (u_short) (work->shade[0].y2 + 6);
        work->shade[0].y3 = (u_short) (work->shade[0].y3 + 6);
        work->shade[1].y0 = (u_short) (work->shade[1].y0 - 6);
        work->shade[1].y1 = (u_short) (work->shade[1].y1 - 6);
        if ((short) work->shade[0].y2 >= 0)
        {
            work->shade[1].r0 = intensity;
            work->shade[1].g0 = intensity;
            work->shade[1].b0 = intensity;
            work->shade[1].r1 = intensity;
            work->shade[1].g1 = intensity;
            work->shade[1].b1 = intensity;
            work->shade[0].r2 = intensity;
            work->shade[0].g2 = intensity;
            work->shade[0].b2 = intensity;
            work->shade[0].r3 = intensity;
            work->shade[0].g3 = intensity;
            work->shade[0].b3 = intensity;
            intensity += 4;
            work->shade[1].y1 = 0U;
            work->shade[1].y0 = 0U;
            work->shade[0].y3 = 0U;
            work->shade[0].y2 = 0U;
            if (intensity >= 0x100)
            {
                running = 0;
            }
        }
        for (i = 0; i < 2; i++)

       {
            GsSortPoly(&work->shade[i], &work->ot[g_draw_side], 0);
        }
        for (i = 0; i < 4; i++)

       {
            GsSortPoly(&work->poly[i], &work->ot[g_draw_side], 1);
        }
        AddPrim(work->ot[g_draw_side].org, &work->mode[g_draw_side]);
        g_fade.rgb[0] = 0U;
        g_fade.rgb[1] = 0U;
        g_fade.rgb[2] = 0U;
        DrawSync(0);
        VSync(2);
        ResetGraph(1);
        GsSwapDispBuff();
        GsSortClear(g_fade.rgb[0], g_fade.rgb[1], g_fade.rgb[2], &work->ot[g_draw_side]);
        GsDrawOt(&work->ot[g_draw_side]);
    } while (running != 0);
    SetDispMask(0);
}
