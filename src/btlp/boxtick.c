/* Persona 1 (JP) - one frame of the box opening or closing.  BTLP only.
 *   0x8007AAB4 BtlBoxTick
 *
 * A unit of its own between the box setup in box.c and the rest of the
 * overlay. It does not come out of the C yet, so BTLP takes it from asm.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <libgte.h>
#include <persona/btlp/box.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/text.h>
#include <persona/btlp/gfx.h>

/* One frame of the box's open or close. Every step ends with the scale at one
   of the two extremes, and the ones that finish drop the step back to zero at
   the shared tail - the ones that are still going return from inside. */
#ifdef NON_MATCHING
void BtlBoxTick(void)
{
    u_char *hold;

    switch (g_btl_box_step) {
    case BTL_BOX_HOLD:
        hold = &g_btl_box_hold;
        (*hold)--;
        if (*hold != 0) {
            return;
        }
        break;
    case BTL_BOX_OPEN_NOW:
        g_btl_box_scale.vx = BTL_BOX_FULL;
        g_btl_box_scale.vy = BTL_BOX_FULL;
        g_btl_box_scale.vz = BTL_BOX_FULL;
        break;
    case BTL_BOX_CLOSE_NOW:
        g_btl_box_scale.vx = 0;
        g_btl_box_scale.vy = 0;
        g_btl_box_scale.vz = 0;
        g_btl_box_flags = 0;
        g_btl_box_step = 0;
        g_btl_text_page = 1;
        BtlTextReset();
        return;
    case BTL_BOX_OPEN:
        g_btl_box_scale.vx = BTL_BOX_FULL;
        g_btl_box_scale.vy += BTL_BOX_RAMP;
        if (g_btl_box_scale.vy < BTL_BOX_FULL) {
            return;
        }
        /* Written again from the branch; the first store is in the original. */
        g_btl_box_scale.vx = BTL_BOX_FULL;
        g_btl_box_scale.vy = BTL_BOX_FULL;
        g_btl_box_scale.vz = BTL_BOX_FULL;
        break;
    case BTL_BOX_CLOSE:
        g_btl_box_scale.vy -= BTL_BOX_RAMP;
        if (g_btl_box_scale.vy >= 0) {
            return;
        }
        g_btl_text_page = 1;
        g_btl_box_scale.vx = 0;
        g_btl_box_scale.vy = 0;
        g_btl_box_scale.vz = 0;
        g_btl_box_flags = 0;
        g_btl_box_step = 0;
        BtlTextReset();
        return;
    case BTL_BOX_ZOOM:
        g_btl_box_scale.vx = g_btl_box_scale.vx / 2 + g_btl_box_scale.vx;
        if (g_btl_box_scale.vx < BTL_BOX_FULL + 1) {
            return;
        }
        g_btl_box_scale.vx = BTL_BOX_FULL;
        g_btl_box_scale.vy = g_btl_box_scale.vy / 2 + g_btl_box_scale.vy;
        if (g_btl_box_scale.vy < BTL_BOX_FULL + 1) {
            g_btl_box_scale.vx = BTL_BOX_FULL;
            return;
        }
        g_btl_box_scale.vy = BTL_BOX_FULL;
        break;
    case BTL_BOX_COLLAPSE_STEP:
        g_btl_box_scale.vy = g_btl_box_scale.vy - g_btl_box_scale.vy / 2;
        if (g_btl_box_scale.vy > BTL_BOX_THIN - 1) {
            return;
        }
        g_btl_box_scale.vy = BTL_BOX_THIN;
        g_btl_box_scale.vx = g_btl_box_scale.vx - g_btl_box_scale.vx / 2;
        if (g_btl_box_scale.vx > 3) {
            g_btl_box_scale.vy = BTL_BOX_THIN;
            return;
        }
        BtlTextReset();
        g_btl_text_page = 1;
        g_btl_box_scale.vx = 0;
        g_btl_box_scale.vy = 0;
        g_btl_box_scale.vz = 0;
        g_btl_box_flags = 0;
        break;
    case 0:
    default:
        return;
    }
    g_btl_box_step = 0;
}
#else
INCLUDE_ASM("btlp/nonmatchings/box", BtlBoxTick);
#endif
