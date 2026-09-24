#ifndef PERSONA_COMMON_BG_H
#define PERSONA_COMMON_BG_H

/* Persona 1 (JP) - the background layers.
 *
 * Six GsBG layers the draw pass hands to GsSortFastBg, each shown while its
 * bit in g_bg_shown is set. Every overlay that draws a field carries its own
 * copy of the set; the maps they draw live in main (g_bg_maps).
 *
 * The first layer's colour doubles as the screen's brightness: the fades step
 * g_bg_layers[0].r and shade everything else by it.
 */
#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>

#define BG_LAYERS 6

extern GsBG    g_bg_layers[BG_LAYERS];
extern u_short g_bg_layer_otz[BG_LAYERS];
extern u_long  g_bg_shown;
extern u_int  *g_bg_maps[];

/* Points the map at the cells to draw - only the pointer is kept - and
   clears the four rows of the map index. */
extern void BgMapInit(void *cells, int arg);
extern void BgMapClearRow(u_short row);

#endif
