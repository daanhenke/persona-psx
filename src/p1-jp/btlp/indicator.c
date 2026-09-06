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
#include <types.h>

#define BTL_INDICATOR_OFF  0
#define BTL_INDICATOR_BAR  1
#define BTL_INDICATOR_ICON 2

extern short g_btl_indicator_mode;
extern short g_btl_indicator_phase;

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
