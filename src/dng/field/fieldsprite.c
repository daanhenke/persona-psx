/* Persona 1 (JP) - setting up one of the field's sprites.  DNG only.
 *   0x8006FCB8 FieldInitSprite
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <persona/dng/field.h>

/* Sprite `i` becomes a w by h cut of texture page `tpage` at (u, v) with
   palette (cx, cy): pivot in the middle, neutral colour, unscaled and
   unrotated. An old-style definition: callers pass plain ints, and the
   routine narrows them itself. */
/* 95.3%: the first seven fields go through one pointer, the rest through
   g_scene afresh, and the stores are in the image's order; sched still puts
   the attribute store after the four stack arguments are read, where the
   image stores it first and reuses its register for u. */
#ifdef NON_MATCHING
void FieldInitSprite(i, w, h, tpage, u, v, cx, cy)
    u_short i, w, h;
    int     tpage;
    u_short u, v, cx, cy;
{
    GsSPRITE *sp;
    sp = g_scene->sprites;
    sp += i;
    sp->w = w;
    sp->h = h;
    sp->attribute = 0x1000000;
    sp->u = u;
    sp->mx = w / 2;
    sp->my = h / 2;
    sp->tpage = tpage;
    g_scene->sprites[i].v = v;
    g_scene->sprites[i].cx = cx;
    g_scene->sprites[i].cy = cy;
    g_scene->sprites[i].r = 0x80;
    g_scene->sprites[i].g = 0x80;
    g_scene->sprites[i].b = 0x80;
    g_scene->sprites[i].rotate = 0;
    g_scene->sprites[i].scalex = 0x1000;
    g_scene->sprites[i].scaley = 0x1000;
}
#else
INCLUDE_ASM("dng/nonmatchings/field/fieldsprite", FieldInitSprite);
#endif
