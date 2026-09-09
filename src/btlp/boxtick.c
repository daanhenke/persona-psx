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

/* Scratch the graphics are unpacked into: the palette first, the frame's tiles
   0x200 bytes in. Reached by hardcoded address rather than through a symbol. */
#define BTL_BOX_CLUT  ((u_char *)0x8014AA00)
#define BTL_BOX_TILES ((u_long *)0x8014AC00)

/* Where each lands in VRAM. The tiles share the message windows' page column,
   so they move with it. */
#define BTL_BOX_PAGE0    11
#define BTL_BOX_PAGE_W   3
#define BTL_BOX_COL      0x40
#define BTL_BOX_TILES_Y  0x188
#define BTL_BOX_TILES_W  0x2C
#define BTL_BOX_TILES_H  0x20
#define BTL_BOX_CLUT_Y   0x1FB
#define BTL_BOX_CLUT_W   0x100

/* Where the box starts from, and how far away it sits. */
#define BTL_BOX_START_X 0x10
#define BTL_BOX_START_Y 0x40
#define BTL_BOX_DIST    100

/* Full size on an axis, how far a ramp step moves, and the sliver the
   collapse stops the height at. */
#define BTL_BOX_FULL 0x1000
#define BTL_BOX_RAMP 0x180
#define BTL_BOX_THIN 0x40

extern u_char *g_btl_box_pack;
extern VECTOR  g_btl_box_pos;
extern SVECTOR g_btl_box_rot;
extern VECTOR  g_btl_box_scale;

extern u_char  g_btl_box_step;
extern u_char  g_btl_box_hold;
extern u_short g_btl_box_flags;

extern void BtlUnpack(u_char *dst, const u_char *src);

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
