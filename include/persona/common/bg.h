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

/* The tiled map layer 4 draws: 16x16-pixel cells, BG_MAP_W across and
   BG_MAP_H down. g_bg_index says which cell goes where. */
#define BG_MAP_W    15
#define BG_MAP_H    4
#define BG_MAP_CELL 16

extern GsMAP   g_bg_map;
extern u_short g_bg_index[BG_MAP_W * BG_MAP_H];

/* The map's cell definitions and its state, both in the work area and
   reached by hardcoded address, so S2D's come out 0x20000 higher on the
   same WORK_BIAS. */
typedef struct {
    /* 0x00 */ u_int  tick;   /* its low bits pick the animated tiles'
                                 palette; BgPanelSet parks it at 0x8000 */
    /* 0x04 */ short  unk04;
    /* 0x06 */ short  unk06;
    /* 0x08 */ short  unk08;  /* BgMapInit's second argument */
    /* 0x0A */ short  unk0A;
    /* 0x0C */ void  *src;    /* what BgMapInit was handed to draw from */
} BgMapState;

#define g_bg_cells ((GsCELL *)(0x800E224C + WORK_BIAS))
#define g_bg_state ((BgMapState *)(0x800E1E4C + WORK_BIAS))

extern void BgMapInit(void *src, short arg);
extern void BgMapClearRow(u_short row);
extern void BgMapSetCell(u_short idx);

#endif
