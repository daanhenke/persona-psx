/* Persona 1 (JP) - the background panel.  ADV @ 0x8007D50C.
 *
 * One of the six background layers, sized to a wide thin strip and placed
 * wherever the caller asks. Putting it up is three things: the map it draws,
 * the layer's rectangle, and its bit in g_bg_shown - the draw pass hands
 * every layer whose bit is set to GsSortFastBg on the next frame.
 *
 * An id of zero takes it down instead, which means blanking the four rows of
 * the map index it occupies rather than clearing the bit; the layer stays
 * shown but draws nothing.
 */
#include <types.h>
#include <libgs.h>

/* Which of the six layers the panel is, and the strip it fills. */
#define PANEL_LAYER 4
#define PANEL_BIT   0x10
#define PANEL_W     0xF0
#define PANEL_H     0x10

/* The rows the map index gives it, and what the tick is put back to. */
#define PANEL_ROWS  4
#define BG_IDLE     0x8000

/* Reached by hardcoded address rather than through the linker symbol. */
#define BG_STATE_AT 0x800E1E4C

extern GsBG   g_bg_layers[];
extern u_long g_bg_shown;
extern u_int *g_bg_maps[];

extern void BgMapInit(u_int *map, int arg);
extern void BgMapClearRow(u_short row);

void BgPanelSet(short id, short x, short y)
{
    u_int *state;

    state = (u_int *)BG_STATE_AT;
    if (id != 0) {
        BgMapInit(g_bg_maps[id], 0);
        g_bg_layers[PANEL_LAYER].w = PANEL_W;
        g_bg_layers[PANEL_LAYER].x = x;
        g_bg_layers[PANEL_LAYER].y = y;
        g_bg_layers[PANEL_LAYER].h = PANEL_H;
        g_bg_shown |= PANEL_BIT;
    } else {
        BgMapClearRow(0);
        BgMapClearRow(1);
        BgMapClearRow(2);
        BgMapClearRow(3);
        *state = BG_IDLE;
    }
}
