#include <persona/s2d/s2d.h>
#include <libetc.h>
#include <string.h>

/* Two ordering tables and copies of the four captured screen strips. */
typedef struct {
    GsOT ot[2];
    GsOT_TAG tags[2][4];
    POLY_FT4 poly[2][4];
    POLY_F4 shade;
} MapFadeWorkspace;

extern MapFadeWorkspace* D_800B85BC;
extern int D_800A4D1C;
extern int D_800A4D20;
extern int g_draw_side;
extern DRAWENV g_s2d_drawenv;
extern void S2dBeginFrame(int);
extern void S2dDrawParty(GsOT*, int);
extern void S2dDrawMapObjs(void);
extern void func_800971D4(void);
extern void func_80089F5C(int);
extern void S2dFrameUnlocked(void);
extern void S2dDraw(void);
extern void S2dLoadLtsAsync2(void);
extern void S2dLoadAdvCmdAsync(void);

/* Keep the original byte-indexing order for the colour stores. Sizes come
   from the workspace layout; the active side is re-read for every store. */
#define FADE_STRIP(work, side, strip) \
    (((POLY_FT4*)((side) * sizeof((work)->poly[0]) + (u_char*)(work) \
       + sizeof((work)->ot) + sizeof((work)->tags)))[strip])

/* Fade in while loading LTS (mode zero), or fade out while loading ADVCMD. */
void func_80092C6C(int mode)
{
    RECT rect;
    MapFadeWorkspace* work;
    int i;
    int color;
    short x0;
    short x1;

    work = D_800B85BC;
    D_800A4D1C = 0;
    D_800A4D20 = 0;
    g_fade.rgb[0] = 0x60;
    g_fade.rgb[1] = 0x60;
    g_fade.rgb[2] = 0x60;
    if (mode == 0) {
        S2dBeginFrame(0);
        S2dDrawParty(&g_ot_map[g_draw_side], 3);
        S2dDrawMapObjs();
        func_800971D4();
        func_80089F5C(0);
    } else {
        S2dFrameUnlocked();
        S2dDraw();
    }
    DrawSync(0);
    for (i = 0; i < 2; i++) {
        work->ot[i].length = 2;
        work->ot[i].org = work->tags[i];
    }
    if (mode == 0) {
        color = 0;
        S2dLoadLtsAsync2();
    } else {
        color = 128;
        S2dLoadAdvCmdAsync();
    }
    rect = g_s2d_drawenv.clip;
    rect.x = 512 - rect.x;
    MoveImage(&rect, 0, 256);
    DrawSync(0);
    SetPolyFT4(&work->poly[0][0]);
    SetSemiTrans(&work->poly[0][0], 0);
    SetShadeTex(&work->poly[0][0], 0);
    work->poly[0][0].u0 = 0;
    work->poly[0][0].v0 = 0;
    work->poly[0][0].u1 = 128;
    work->poly[0][0].v1 = 0;
    work->poly[0][0].u2 = 0;
    work->poly[0][0].v2 = 240;
    work->poly[0][0].u3 = 128;
    work->poly[0][0].v3 = 240;
    i = 0;
    x1 = -128;
    x0 = -256;
    for (; i < 4; i++) {
        memmove(&work->poly[0][i], &work->poly[0][0], sizeof(POLY_FT4));
        work->poly[0][i].y0 = -120;
        work->poly[0][i].y1 = -120;
        work->poly[0][i].y2 = 120;
        work->poly[0][i].x0 = x0;
        work->poly[0][i].x1 = x1;
        work->poly[0][i].x2 = x0;
        work->poly[0][i].x3 = x1;
        work->poly[0][i].y3 = 120;
        work->poly[0][i].tpage = GetTPage(2, 0, i * 128, 256);
        x1 += 128;
        x0 += 128;
    }
    __builtin_memcpy(work->poly[1], work->poly[0], sizeof(work->poly[0]));
    if (mode == 0) {
        SetPolyF4(&work->shade);
        work->shade.r0 = 0;
        work->shade.g0 = 0;
        work->shade.b0 = 0;
        work->shade.x0 = -256;
        work->shade.y0 = -120;
        work->shade.x1 = 256;
        work->shade.y1 = -120;
        work->shade.x2 = -256;
        work->shade.y2 = 120;
        work->shade.x3 = 256;
        work->shade.y3 = 120;
        DrawPrim(&work->shade);
        DrawSync(0);
    }
    SetDispMask(1);
    while (mode == 0 ? color < 128 : color >= 7) {
        g_draw_side = GsGetActiveBuff();
        GsClearOt(0, 0, &work->ot[g_draw_side]);
        if (mode == 0) {
            color += 6;
        } else {
            color -= 6;
        }
        for (i = 0; i < 4; i++) {
            FADE_STRIP(work, g_draw_side, i).r0 = color;
            FADE_STRIP(work, g_draw_side, i).g0 = color;
            FADE_STRIP(work, g_draw_side, i).b0 = color;
            AddPrim(work->ot[g_draw_side].org, &work->poly[g_draw_side][i]);
        }
        g_fade.rgb[0] = 0;
        g_fade.rgb[1] = 0;
        g_fade.rgb[2] = 0;
        DrawSync(0);
        VSync(0);
        ResetGraph(1);
        GsSwapDispBuff();
        GsSortClear(g_fade.rgb[0], g_fade.rgb[1], g_fade.rgb[2], &work->ot[g_draw_side]);
        GsDrawOt(&work->ot[g_draw_side]);
    }
}
