/* Persona 1 (JP) - the play-time clock in the money box.  ADV @ 0x8007CED4.
 *
 * The box BgBoxShow puts up is twelve cells across; its third row carries the
 * time played as HH:MM from the fifth cell on. The hours are right-aligned
 * into two cells, the minutes always take two with a leading zero, and the
 * colon between them blinks - shown for the second half of each second of
 * frames, blank for the first.
 *
 * Every menu that redraws the field status calls this, so the clock keeps
 * running behind them.
 */
#include <decomp/types.h>

/* Where the clock sits in the box's cells, and what it is drawn with. */
#define BOX_CELLS_W   0xC
#define CLOCK_AT      (2 * BOX_CELLS_W + 5)
#define CLOCK_W       5
#define CLOCK_HOURS   1          /* the last of the hours' two cells  */
#define CLOCK_COLON   2
#define CLOCK_MINUTES 4          /* the last of the minutes' two cells */
#define GLYPH_DIGIT0  0xC0
#define GLYPH_COLON   0xCB

/* Half of the sixty frames a second. */
#define BLINK_FRAMES  30

/* The hours are reached by address and the rest by name, as BtlClockTick
   reaches them. */
#define g_playtime_hours ((u_char *)0x801F29BC)

extern u_char g_playtime_min;
extern u_char g_playtime_frame;

extern short  g_panel_cells[];
extern u_char g_hud_digits[];

extern short FormatDecimal(u_int value, u_char *dst, u_short width);
extern void  TileMapWriteRowRev(const u_char *src, short *dst, int base,
                                u_short count);
extern void  TileMapFillRect(short *dst, short value, u_short w, u_short h,
                             u_short stride);

void DrawStatusHud(void)
{
    u_char *hours;
    short  *cells;
    int     n;

    hours = g_playtime_hours;
    cells = &g_panel_cells[CLOCK_AT];
    TileMapFillRect(cells, 0, CLOCK_W, 1, BOX_CELLS_W);

    n = FormatDecimal(g_playtime_min, g_hud_digits, 2);
    if (n == 1) {
        n = 2;
    }
    TileMapWriteRowRev(g_hud_digits, &cells[CLOCK_MINUTES], GLYPH_DIGIT0, n);

    n = FormatDecimal(*hours, g_hud_digits, 2);
    TileMapWriteRowRev(g_hud_digits, &cells[CLOCK_HOURS], GLYPH_DIGIT0, n);

    if (g_playtime_frame < BLINK_FRAMES) {
        cells[CLOCK_COLON] = 0;
    } else {
        cells[CLOCK_COLON] = GLYPH_COLON;
    }
}
