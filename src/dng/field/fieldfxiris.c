/* Persona 1 (JP) - the closing-circle battle transition.  DNG only.
 *   0x800722A8 FieldIrisFxBegin
 *   0x80072310 FieldIrisFxRun
 *   0x8007245C FieldIrisFxStep
 *   0x80072610 FieldIrisFxSetup
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <persona/dng/field.h>

#define STRIP_COLUMNS 160
#define STRIP_MIDDLE  (STRIP_COLUMNS / 2)

/* As FieldFxBegin, for the closing circle. */
void FieldIrisFxBegin(void)
{
    func_80065978();
    do {
        func_80065978();
    } while (g_draw_buf != 0);
    FieldIrisFxSetup();
    VSync(3);
    FieldClockTick(3);
    GsDefDispBuff(320, 256, 0, 240);
}

/* Closes a circle of radius 200 down to nothing, two pixels a frame: each
   pair of columns either side of the middle is scaled to the circle's
   height at that distance. */
/* 85.1%: the image keeps the right-hand column as an int index counting
   down from a register holding 79 and multiplies it out at each store;
   loop.c strength-reduces an int index here, so it is a u_short, which
   costs a mask at each use. The clamp's branches are also laid out the
   other way round. */
#ifdef NON_MATCHING
void FieldIrisFxRun(void)
{
    int r, rr, i, x, h;
    u_short j;

    for (r = 200; r >= 0; r -= 2) {
        rr = r * r;
        for (i = 0, x = 0, j = STRIP_MIDDLE - 1; i < STRIP_MIDDLE; i++, x += 2, j--) {
            if (x > r) {
                g_scene->sprites[STRIP_MIDDLE + i].scaley = 0;
                g_scene->sprites[j].scaley = 0;
                continue;
            }
            h = csqrt((rr - x * x) << 12) / 120;
            if (h >= 0x1000) {
                h = 0x1000;
            } else if (h < 0) {
                g_scene->sprites[STRIP_MIDDLE + i].scaley = 0;
                g_scene->sprites[j].scaley = 0;
                continue;
            }
            g_scene->sprites[STRIP_MIDDLE + i].scaley = h;
            g_scene->sprites[j].scaley = h;
        }
        FieldIrisFxStep();
    }
}
#else
INCLUDE_ASM("dng/nonmatchings/field/fieldfxiris", FieldIrisFxRun);
#endif

/* One frame of the circle: every column a little brighter. */
void FieldIrisFxStep(void)
{
    int i;

    g_draw_buf = GsGetActiveBuff();
    GsSetWorkBase(g_scene->packets[g_draw_buf]);
    GsClearOt(0, 0, &g_scene->ot[g_draw_buf]);
    for (i = 0; i < STRIP_COLUMNS; i++) {
        g_scene->sprites[i].r++;
        g_scene->sprites[i].g++;
        g_scene->sprites[i].b++;
        GsSortSprite(&g_scene->sprites[i], &g_scene->ot[g_draw_buf], 0);
    }
    VSync(2);
    FieldClockTick(2);
    GsSwapDispBuff();
    GsSortClear(0, 0, 0, &g_scene->ot[g_draw_buf]);
    GsDrawOt(&g_scene->ot[g_draw_buf]);
}

/* The backdrop strip again, pivoting on the screen's middle line so it can
   be squashed towards it. */
void FieldIrisFxSetup(void)
{
    int       i;
    GsSPRITE *sp;

    for (i = 0; i < STRIP_COLUMNS; i++) {
        FieldInitSprite(i, 2, 240, i / 32, (i & 31) * 2, 0, 0, 0);
        sp = g_scene->sprites;
        sp += i;
        sp->my = 120;
        sp->attribute |= 0x2000000;
        sp->r = 0x80;
        sp->rotate = 0;
        g_scene->sprites[i].g = 0x80;
        g_scene->sprites[i].b = 0x80;
        g_scene->sprites[i].x = i * 2 - (STRIP_COLUMNS - 1);
        g_scene->sprites[i].scaley = 0x1000;
        g_scene->sprites[i].y = 0;
    }
}
