/* Persona 1 (JP) - the fading battle transition.  DNG only.
 *   0x80071B94 FieldFadeFxBegin
 *   0x80071BFC FieldFadeFxRun
 *   0x80071C50 FieldFadeFxStep
 *   0x80071E04 FieldFadeFxSetup
 */
#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <persona/dng/field.h>

/* As FieldFxBegin, for the transition that fades the backdrop strip. */
void FieldFadeFxBegin(void)
{
    func_80065978();
    do {
        func_80065978();
    } while (g_draw_buf != 0);
    FieldFadeFxSetup();
    VSync(3);
    FieldClockTick(3);
    GsDefDispBuff(320, 256, 0, 240);
}

/* Steps the fade until the strip's first column has gone dark. */
void FieldFadeFxRun(void)
{
    while (g_scene->sprites[0].r != 0) {
        FieldFadeFxStep();
    }
}

/* One frame of the fade: every tile of the grid a little darker, drawn on
   its own over the last frame. */
void FieldFadeFxStep(void)
{
    int i;

    g_draw_buf = GsGetActiveBuff();
    GsSetWorkBase(g_scene->packets[g_draw_buf]);
    GsClearOt(0, 0, &g_scene->ot[g_draw_buf]);
    for (i = 0; i < 80; i++) {
        g_scene->sprites[i].r -= 4;
        g_scene->sprites[i].g -= 4;
        g_scene->sprites[i].b -= 4;
        GsSortFastSprite(&g_scene->sprites[i], &g_scene->ot[g_draw_buf], 0);
    }
    VSync(2);
    FieldClockTick(2);
    GsSwapDispBuff();
    GsSortClear(0, 0, 0, &g_scene->ot[g_draw_buf]);
    GsDrawOt(&g_scene->ot[g_draw_buf]);
}

/* Covers the screen with a grid of 32 by 32 sprites, ten across and eight
   down, cut from the backdrop's pages two columns to a page. */
void FieldFadeFxSetup(void)
{
    int row, col, n;

    for (row = 0; row < 8; row++) {
        for (col = 0; col < 10; col++) {
            n = col + row * 10;
            FieldInitSprite(n, 32, 32, col / 2, (col & 1) * 32, row * 32, 0, 0);
            g_scene->sprites[n].x = col * 32 - 160;
            g_scene->sprites[n].y = row * 32 - 120;
            g_scene->sprites[n].attribute |= 0x2000000;
        }
    }
}
