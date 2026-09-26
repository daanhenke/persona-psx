/* Persona 1 (JP) - the field menu screen.  DNG's copy.
 *   0x8008A2DC MenuScreenDraw
 *
 * The whole screen in one call: the window frame in one piece, the menu's
 * backdrop blitted over it, then the money box, the status HUD and the five
 * party slots. ADV's copy (src/adv/gfx/menuscreen.c) lays the frame down a
 * row routine per column; this one, like S2D's, hands it to
 * TileMapDrawWindow.
 */
#include <decomp/types.h>

/* The frame, in cells of a layer 0x28 wide. */
#define FRAME_ROWS   8
#define FRAME_COLS   0x26
#define LAYER_STRIDE 0x28

/* The frame starts one cell in; the backdrop goes over the whole layer. */
#define g_frame_at ((short *)0x800EE182)
#define g_layer_at ((short *)0x800EE180)

extern const u_short g_menu_bg_rle[];

extern void TileMapDrawWindow(short *dst, u_char w, u_char h, u_char stride);
extern void TileMapBlitRle(const u_short *src, short *dst, u_short stride);
extern void BgBoxShow(void);
extern void DrawStatusHud(void);
extern void DrawPartySlotStatus(int slot, int kind);

void MenuScreenDraw(void)
{
    TileMapDrawWindow(g_frame_at, FRAME_COLS, FRAME_ROWS, LAYER_STRIDE);
    TileMapBlitRle(g_menu_bg_rle, g_layer_at, LAYER_STRIDE);
    BgBoxShow();
    DrawStatusHud();
    DrawPartySlotStatus(0, 0);
    DrawPartySlotStatus(1, 0);
    DrawPartySlotStatus(2, 0);
    DrawPartySlotStatus(3, 0);
    DrawPartySlotStatus(4, 0);
}
