/* Persona 1 (JP) - the small indicator the battle screen carries.
 *   BTLP @ 0x8007EF08 BtlIndicatorIcon, 0x8007EF24 BtlIndicatorBar,
 *         0x8007EF38 BtlIndicatorClear
 *
 * Three modes, and the one reader - BtlDrawIndicator - shows what they mean.
 * Mode 0 blanks eleven cells. Mode 1 fills those cells from a ramp table and
 * runs a phase 0..0x37. Mode 2 draws a 0x20 by 0x18 sprite whose u flips on bit
 * 5 of that phase, so it blinks between two frames. 0x8006A170 toggles between
 * the last two on a button press while the battle is waiting.
 *
 * The mode word used to be called g_btl_bgm_pending, on the reading that it
 * held a track somebody had asked for. It does not: its only reader builds a
 * SPRT and a row of glyph cells and never touches the sound driver.
 */
#include <decomp/types.h>
#include <decomp/libc.h>
#include <libgte.h>
#include <libgpu.h>
#include <persona/btlp/battle.h>

#define BTL_INDICATOR_OFF  0
#define BTL_INDICATOR_BAR  1
#define BTL_INDICATOR_ICON 2

/* Cells the bar is made of, and what a blank one holds. */
#define BTL_INDICATOR_CELLS 11
#define BTL_INDICATOR_BLANK 0xCE

/* How far the bar's phase runs before it comes round again. */
#define BTL_INDICATOR_PERIOD 0x38

/* The icon: where it sits relative to the HUD, how big it is, which corner of
   the page it comes from, and the bit of the phase that flips between the two
   frames. */
#define BTL_ICON_DX    0x70
#define BTL_ICON_DY    8
#define BTL_ICON_W     0x20
#define BTL_ICON_H     0x18
#define BTL_ICON_U     0x20
#define BTL_ICON_V     0x80
#define BTL_ICON_BLINK 0x20
#define BTL_ICON_CLUT  0x1F3
#define BTL_ICON_TP    1
#define BTL_ICON_PAGE_X 0x350
#define BTL_ICON_PAGE_Y 0x180

/* Where the eleven cells are uploaded to. */
#define BTL_INDICATOR_VRAM_X 0x20
#define BTL_INDICATOR_VRAM_Y 0x1FA

extern short g_btl_indicator_mode;
extern u_short g_btl_indicator_phase;

extern short  g_btl_indicator_cells[];
extern short  g_btl_indicator_ramp[];
extern u_short g_btl_hud_x;
extern u_short g_btl_hud_y;
extern char  *g_btl_prim_next;
extern u_long g_btl_ot[][3];
extern int    g_btl_ot_index;

void BtlIndicatorIcon(void)
{
    g_btl_indicator_mode = BTL_INDICATOR_ICON;
    g_btl_indicator_phase = 0;
}

void BtlIndicatorBar(void)
{
    g_btl_indicator_mode = BTL_INDICATOR_BAR;
}

void BtlIndicatorClear(void)
{
    g_btl_indicator_mode = BTL_INDICATOR_OFF;
}

/* One frame of whichever form the indicator is in. The bar's cells are filled
   back to front from a ramp the phase walks along, so the lit end travels; the
   icon is one sprite whose texture corner flips on a bit of the same phase, so
   it blinks between two frames. Either way the eleven cells are uploaded at
   the end, blank or not. */
void BtlDrawIndicator(void)
{
    SPRT    sprt;
    DR_MODE mode;
    short  *cell;
    int     u;
    int     x;
    short   blank;
    int     i;

    switch (g_btl_indicator_mode) {
    case BTL_INDICATOR_OFF:
        blank = BTL_INDICATOR_BLANK;
        i     = BTL_INDICATOR_CELLS - 1;
        cell  = &g_btl_indicator_cells[BTL_INDICATOR_CELLS - 1];
        do {
            *cell = blank;
            i--;
            cell--;
        } while (i >= 0);
        g_btl_indicator_phase = 0;
        break;

    case BTL_INDICATOR_BAR:
        i = 0;
        do {
            g_btl_indicator_cells[(BTL_INDICATOR_CELLS - 1) - i] =
                g_btl_indicator_ramp[i + (g_btl_indicator_phase >> 1)];
            i++;
        } while (i < BTL_INDICATOR_CELLS);
        g_btl_indicator_phase =
            (g_btl_indicator_phase + 1) % BTL_INDICATOR_PERIOD;
        break;

    case BTL_INDICATOR_ICON:
        SetSprt(&sprt);
        SetSemiTrans(&sprt, 0);
        SetShadeTex(&sprt, 1);
        sprt.v0 = BTL_ICON_V;
        sprt.w  = BTL_ICON_W;
        x       = g_btl_hud_x;
        sprt.x0 = x + BTL_ICON_DX;
        sprt.h  = BTL_ICON_H;
        u       = (g_btl_indicator_phase & BTL_ICON_BLINK) + BTL_ICON_U;
        sprt.y0 = g_btl_hud_y + BTL_ICON_DY;
        sprt.u0 = u;
        sprt.clut = GetClut(0, BTL_ICON_CLUT);
        SetDrawMode(&mode, 0, 0,
                    GetTPage(BTL_ICON_TP, 0, BTL_ICON_PAGE_X, BTL_ICON_PAGE_Y),
                    0);
        memcpy(g_btl_prim_next, &sprt, sizeof(SPRT));
        AddPrim(g_btl_ot[g_btl_ot_index], g_btl_prim_next);
        g_btl_prim_next += sizeof(SPRT);
        memcpy(g_btl_prim_next, &mode, sizeof(DR_MODE));
        AddPrim(g_btl_ot[g_btl_ot_index], g_btl_prim_next);
        g_btl_prim_next += sizeof(DR_MODE);
        g_btl_indicator_phase++;
        break;
    }
    BtlQueueVramLoad(g_btl_indicator_cells, BTL_INDICATOR_VRAM_X,
                     BTL_INDICATOR_VRAM_Y, BTL_INDICATOR_CELLS, 1);
}
