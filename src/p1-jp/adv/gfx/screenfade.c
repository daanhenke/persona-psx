/* Persona 1 (JP) - three of the ADV overlay's full-screen effects.
 *
 *   AdvEffectFadeRed   @ 0x8008695C
 *   AdvEffectFadeGreen @ 0x80086AD4
 *   AdvEffectFadeBlue  @ 0x80086C4C
 *
 * AdvRunFrame draws one effect a frame, picked by g_adv_effect; these are its
 * cases 1, 2 and 3. All three are the same picture in a different colour: a
 * band fading in from the top of the screen and another fading out at the
 * bottom, both scaled by the overlay's fade level so the effect comes up and
 * goes down with a screen fade.
 *
 * Red and green are drawn with gouraud lines, the same colour at both ends -
 * which is what fixes GsGLINE's layout, since each writes its one channel to a
 * pair of offsets three apart. Blue is drawn with plain lines instead, 116 to a
 * band rather than 120, and sits four scanlines lower; the two forms are not
 * otherwise different, and nothing here says why.
 *
 * The lines share one buffer with DrawBackdrop, which is why only one of them
 * can be on screen at a time.
 */
#include <types.h>
#include <libgs.h>

#define g_glines ((GsGLINE *)0x800ED180)
#define g_lines  ((GsLINE *)0x800ED180)

extern GsOT     g_ot[];
extern int      g_ot_index;
extern GsBG    g_bg_layers[];

#define SCREEN_W 0x140
#define OT_BACK  0x400

/* The gouraud pair: 120 lines a band, the lower one starting at y = 128. */
#define GBAND    120
#define GLOWER_Y 0x80

/* The plain one: 116 a band, four scanlines lower, but the fade still runs
   over 120 so the band stops four short of black rather than reaching it. */
#define LBAND    116
#define LLOWER_Y 0x84

void AdvEffectFadeRed(void)
{
    GsGLINE *l;
    int shade;
    int i;

    l = g_glines;
    for (i = 0; i < GBAND; i++) {
        l->attribute = 0;
        l->x1 = SCREEN_W;
        l->x0 = 0;
        l->y0 = i;
        l->y1 = i;
        shade = (GBAND - i) * g_bg_layers[0].r / 128;
        l->r0 = (l->r1 = shade);
        l->g0 = (l->g1 = 0);
        l->b0 = (l->b1 = 0);
        GsSortGLine(l, &g_ot[g_ot_index], OT_BACK);
        l++;
    }
    for (i = 0; i < GBAND; i++) {
        l->attribute = 0;
        l->x1 = SCREEN_W;
        l->x0 = 0;
        l->y0 = i + GLOWER_Y;
        l->y1 = i + GLOWER_Y;
        shade = i * g_bg_layers[0].r / 128;
        l->r0 = (l->r1 = shade);
        l->g0 = (l->g1 = 0);
        l->b0 = (l->b1 = 0);
        GsSortGLine(l, &g_ot[g_ot_index], OT_BACK);
        l++;
    }
}

void AdvEffectFadeGreen(void)
{
    GsGLINE *l;
    int shade;
    int i;

    l = g_glines;
    for (i = 0; i < GBAND; i++) {
        l->attribute = 0;
        l->x1 = SCREEN_W;
        l->x0 = 0;
        l->y0 = i;
        l->y1 = i;
        shade = (GBAND - i) * g_bg_layers[0].r / 128;
        l->r0 = (l->r1 = 0);
        l->g0 = (l->g1 = shade);
        l->b0 = (l->b1 = 0);
        GsSortGLine(l, &g_ot[g_ot_index], OT_BACK);
        l++;
    }
    for (i = 0; i < GBAND; i++) {
        l->attribute = 0;
        l->x1 = SCREEN_W;
        l->x0 = 0;
        l->y0 = i + GLOWER_Y;
        l->y1 = i + GLOWER_Y;
        shade = i * g_bg_layers[0].r / 128;
        l->r0 = (l->r1 = 0);
        l->g0 = (l->g1 = shade);
        l->b0 = (l->b1 = 0);
        GsSortGLine(l, &g_ot[g_ot_index], OT_BACK);
        l++;
    }
}

void AdvEffectFadeBlue(void)
{
    GsLINE *l;
    int shade;
    int i;

    l = g_lines;
    for (i = 0; i < LBAND; i++) {
        l->attribute = 0;
        l->x1 = SCREEN_W;
        l->x0 = 0;
        l->y0 = i;
        l->y1 = i;
        shade = (GBAND - i) * g_bg_layers[0].r / 128;
        l->r = 0;
        l->g = 0;
        l->b = shade;
        GsSortLine(l, &g_ot[g_ot_index], OT_BACK);
        l++;
    }
    for (i = 0; i < LBAND; i++) {
        l->attribute = 0;
        l->x1 = SCREEN_W;
        l->x0 = 0;
        l->y0 = i + LLOWER_Y;
        l->y1 = i + LLOWER_Y;
        shade = i * g_bg_layers[0].r / 128;
        l->r = 0;
        l->g = 0;
        l->b = shade;
        GsSortLine(l, &g_ot[g_ot_index], OT_BACK);
        l++;
    }
}
