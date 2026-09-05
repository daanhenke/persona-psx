/* Persona 1 (JP) - the background the menus are drawn over.
 *
 * S2D copy; see src/p1-jp/common/gfx/backdrop.c for the shared original.
 *   S2D @ 0x8007BAF8
 *
 * 155 lines, sorted into the OT ahead of everything else. The first 120 are a
 * blue gradient down the top half of the screen, brightest at the top and
 * fading to nothing at line 120; the rest is a 16-pixel grid, fifteen
 * horizontal and twenty vertical, in grey.
 *
 * Every colour is scaled by the overlay's fade level - the r of the first fade
 * sprite - so the whole backdrop comes up and goes down with a screen fade
 * rather than being drawn and then covered.
 *
 * The line buffer sits 0x20000 above the DNG and ADV copies.
 */
#include <types.h>
#include <libgs.h>

/* Reached by hardcoded address. 155 GsLINEs. */
#define g_backdrop_lines ((GsLINE *)0x8010D180)

extern GsSPRITE g_fade_sprites[];
extern GsOT     g_ot[];
extern int      g_ot_index;

#define SCREEN_W 0x140
#define SCREEN_H 0xF0
#define GRID     16

#define GRADIENT_H 120     /* the gradient's last line, and its divisor    */
#define OT_BACK    0x400   /* behind the grid, which sits at 0x3FF         */
#define OT_GRID    0x3FF

void DrawBackdrop(void)
{
    GsLINE *l;
    /* One local for the gradient's blue and then for the grid's grey: both
       want the same register, and two locals do not compile to this. */
    int shade;
    int i;

    l = g_backdrop_lines;
    for (i = 0; i < GRADIENT_H; i++) {
        l->attribute = 0;
        l->x1 = SCREEN_W;
        l->x0 = 0;
        l->y0 = i;
        l->y1 = i;
        shade = (GRADIENT_H - i) * g_fade_sprites[0].r / 64;
        l->r = 0;
        l->g = 0;
        l->b = shade;
        GsSortLine(l, &g_ot[g_ot_index], OT_BACK);
        l++;
    }
    shade = g_fade_sprites[0].r >> 1;
    for (i = 0; i < 15; i++) {
        l->attribute = 0;
        l->x0 = 0;
        l->x1 = SCREEN_W;
        l->y0 = (l->y1 = i * GRID);
        l->r = shade;
        l->g = shade;
        l->b = shade;
        GsSortLine(l, &g_ot[g_ot_index], OT_GRID);
        l++;
    }
    for (i = 0; i < 20; i++) {
        l->attribute = 0;
        l->x0 = (l->x1 = i * GRID + 4);
        l->y0 = 0;
        l->y1 = SCREEN_H;
        l->r = shade;
        l->g = shade;
        l->b = shade;
        GsSortLine(l, &g_ot[g_ot_index], OT_GRID);
        l++;
    }
}
