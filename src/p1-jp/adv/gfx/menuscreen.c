/* Persona 1 (JP) - the field menu screen.  ADV @ 0x8007B89C.
 *
 * The whole screen in one call. Eight rows of window frame go down first,
 * across 0x26 columns - the top row and the bottom row are drawn by their own
 * routines and everything between by a third - and then the menu's backdrop is
 * blitted over them. After that come the money box, the status HUD, and the
 * five party slots.
 *
 * Each row routine is called once per column rather than once per row, so the
 * width it is handed is the whole run and the calls walk it.
 */
#include <types.h>

/* The frame, in cells of a layer 0x28 wide. */
#define FRAME_ROWS   8
#define FRAME_COLS   0x26
#define LAYER_STRIDE 0x28

/* The frame starts one cell in; the backdrop goes over the whole layer. */
#define g_frame_at ((short *)0x800EE182)
#define g_layer_at ((short *)0x800EE180)

/* Party slots down the side. */
#define PARTY_SLOTS 5

extern const u_short g_menu_bg_rle[];

extern void WindowRowTop(short *dst, u_short cols);
extern void WindowRowMiddle(short *dst, u_short cols);
extern void WindowRowBottom(short *dst, u_short cols);
extern void TileMapBlitRle(const u_short *src, short *dst, u_short stride);
extern void BgBoxShow(void);
extern void DrawStatusHud(void);
extern void DrawPartySlotStatus(int slot, int kind);

void MenuScreenDraw(void)
{
    short *dst;
    u_char row;
    u_char col;
    u_char kind;

    dst = g_frame_at;
    row = 0;
    do {
        col = 0;
        kind = row;
        do {
            if (kind == 0) {
                WindowRowTop(dst, FRAME_COLS);
            } else if (kind == FRAME_ROWS - 1) {
                WindowRowBottom(dst, FRAME_COLS);
            } else {
                WindowRowMiddle(dst, FRAME_COLS);
            }
            col++;
        } while (col < FRAME_COLS);
        row++;
        dst += LAYER_STRIDE;
    } while (row < FRAME_ROWS);

    TileMapBlitRle(g_menu_bg_rle, g_layer_at, LAYER_STRIDE);
    BgBoxShow();
    DrawStatusHud();
    DrawPartySlotStatus(0, 0);
    DrawPartySlotStatus(1, 0);
    DrawPartySlotStatus(2, 0);
    DrawPartySlotStatus(3, 0);
    DrawPartySlotStatus(4, 0);
}
