/* Persona 1 (JP) - the menu screens' background layer layouts.
 *
 * Compiled into ADV and S2D rather than called across the boundary (DNG
 * carries the same three in ui/menucells.c):
 *                  DNG         ADV         S2D
 *   BgLineShow     0x80092E10  0x8008ED70  0x80083324
 *   BgLineHide     0x80092E44  0x8008EDA4  0x80083358
 *   MenuSetLayers  0x80092E5C  0x8008EDBC  0x80083370
 *
 * A menu page is up to four background layers placed over the screen. Each
 * layout is a record of which layers show and where each of the four sits;
 * MenuSetLayers puts one in place and rewinds every layer's scroll, the
 * fourth to its resting 4,4. The fifth layer is the one-line banner the two
 * small routines put up and take down.
 */
#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>

typedef struct {
    short x, y, w, h;
} LayerRect;

typedef struct {
    u_char    shown;       /* g_bg_shown's bits */
    u_char    pad01;
    LayerRect layer[4];
} MenuLayout;              /* 34 bytes */

#define LINE_LAYER 4

extern GsBG       g_bg_layers[];
extern u_long     g_bg_shown;
extern short      g_map_scroll_x, g_map_scroll_y;
extern short      g_header_scroll_x, g_header_scroll_y;
extern MenuLayout g_menu_layer_defs[];

void BgLineShow(void)
{
    g_bg_layers[LINE_LAYER].x = 0x10;
    g_bg_layers[LINE_LAYER].y = 8;
    g_bg_layers[LINE_LAYER].w = 0xF0;
    g_bg_layers[LINE_LAYER].h = 0x10;
}

void BgLineHide(void)
{
    g_bg_layers[LINE_LAYER].w = 0;
    g_bg_layers[LINE_LAYER].h = 0;
}

void MenuSetLayers(short n)
{
    MenuLayout *m;

    m = &g_menu_layer_defs[n];
    g_bg_shown = m->shown;
    g_bg_layers[0].x = m->layer[0].x;
    g_bg_layers[0].y = m->layer[0].y;
    g_bg_layers[0].w = m->layer[0].w;
    g_bg_layers[0].h = m->layer[0].h;
    g_bg_layers[0].scrollx = 0;
    g_bg_layers[0].scrolly = 0;
    g_bg_layers[1].x = m->layer[1].x;
    g_bg_layers[1].y = m->layer[1].y;
    g_bg_layers[1].w = m->layer[1].w;
    g_bg_layers[1].h = m->layer[1].h;
    g_bg_layers[1].scrollx = 0;
    g_bg_layers[1].scrolly = 0;
    g_bg_layers[2].x = m->layer[2].x;
    g_bg_layers[2].y = m->layer[2].y;
    g_bg_layers[2].w = m->layer[2].w;
    g_bg_layers[2].h = m->layer[2].h;
    g_bg_layers[2].scrollx = 0;
    g_bg_layers[2].scrolly = 0;
    g_bg_layers[3].x = m->layer[3].x;
    g_bg_layers[3].y = m->layer[3].y;
    g_bg_layers[3].w = m->layer[3].w;
    g_bg_layers[3].h = m->layer[3].h;
    g_map_scroll_x = 0;
    g_map_scroll_y = 0;
    g_header_scroll_x = 0;
    g_header_scroll_y = 0;
    g_bg_layers[3].scrollx = 4;
    g_bg_layers[3].scrolly = 4;
}
