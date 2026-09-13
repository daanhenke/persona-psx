/* Persona 1 (JP) - the debug grid's contents.  BTLP only.
 *   0x80097974 BtlLoadDebugGrid
 *
 * Seventy-five bytes copied over the cells the debug overlay draws. The
 * source is the block ovl_btlp_entry clears alongside them as the overlay
 * opens, so this is putting back whatever was last laid out rather than
 * building anything.
 */
#include <decomp/types.h>
#include <decomp/libc.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/round.h>

void BtlLoadDebugGrid(void)
{
    memcpy(g_btl_debug_grid_cells, g_btl_reach, REACH_CELLS);
}
