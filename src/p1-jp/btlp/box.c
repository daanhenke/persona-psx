/* Persona 1 (JP) - the battle's message box.  BTLP only.
 *   0x8007A964 BtlBoxOpen   0x8007AA5C BtlBoxClose
 *   0x8007F3C8 BtlCloseMessage  0x8007AAB4 BtlBoxTick
 *
 * The framed panel the battle's text appears in. It is drawn as a row of
 * textured quads through the GTE, so opening and closing it is entirely a
 * matter of moving the scale its matrix is built from: the box grows out of
 * nothing and collapses back into it.
 *
 * Callers put the words in first and the frame around them second, and take
 * them down in the other order:
 *
 *     BtlTextOpen(script, 0x28, 0x92);
 *     BtlBoxOpen(17, 0xA0, 0x92, 0);
 *     BtlWaitAnyKey();
 *     ...
 *     BtlPanelClose(); BtlBoxClose(); BtlSeqClear();
 *
 * Which is why the box is given a centre and a width in columns while the text
 * is given its own top-left corner.
 */
#include <types.h>
#include <psyq/libgte.h>
#include <persona/btlp/box.h>

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

extern u_char  g_btl_fast_anim;
extern int     g_btl_text_page;
extern u_char *g_btl_box_pack;
extern VECTOR  g_btl_box_pos;
extern SVECTOR g_btl_box_rot;
extern VECTOR  g_btl_box_scale;

extern u_char  g_btl_box_step;
extern u_char  g_btl_box_hold;
extern u_short g_btl_box_flags;

extern void BtlUnpack(u_char *dst, const u_char *src);
extern void BtlTextReset(void);
/* No prototype: this file hands it plain ints and lets the callee narrow. */
extern void BtlQueueVramLoad();

/* Reading the graphics in is the one thing that does not happen every time the
   box opens; bit 0x20 of the flags is what remembers it has been done. */
void BtlBoxLoad(void)
{
    BtlUnpack(BTL_BOX_CLUT, g_btl_box_pack);
    BtlQueueVramLoad(BTL_BOX_TILES,
                     (g_btl_text_page * BTL_BOX_PAGE_W + BTL_BOX_PAGE0) *
                         BTL_BOX_COL,
                     BTL_BOX_TILES_Y, BTL_BOX_TILES_W, BTL_BOX_TILES_H);
    BtlQueueVramLoad((u_long *)BTL_BOX_CLUT, 0, BTL_BOX_CLUT_Y,
                     BTL_BOX_CLUT_W, 1);
    g_btl_box_pos.vz = BTL_BOX_DIST;
    g_btl_box_scale.vx = BTL_BOX_START_X;
    g_btl_box_scale.vy = BTL_BOX_START_Y;
    g_btl_box_rot.vx = 0;
    g_btl_box_rot.vy = 0;
    g_btl_box_rot.vz = 0;
    g_btl_box_pos.vx = 0;
    g_btl_box_pos.vy = 0;
    g_btl_box_scale.vz = 0;
    g_btl_box_step = 0;
    g_btl_box_flags = BTL_BOX_LOADED;
}

/* cols is clamped rather than trusted: the frame is drawn from a fixed set of
   column tiles and there is no tile for a box narrower than three or wider
   than seventeen. */
void BtlBoxOpen(short cols, short x, short y, int style)
{
    u_short *flags;
    int      n;

    flags = &g_btl_box_flags;
    if ((*flags & BTL_BOX_LOADED) == 0) {
        BtlBoxLoad();
        *flags |= BTL_BOX_FRAME | BTL_BOX_SLIDE;
    }
    g_btl_box_step = BTL_BOX_OPEN;
    *flags = (*flags & ~BTL_BOX_STYLE) | (style << BTL_BOX_STYLE_SHIFT);
    if (g_btl_fast_anim != 0) {
        g_btl_box_step = BTL_BOX_OPEN_NOW;
    }
    n = cols;
    if (n > BTL_BOX_COLS_MAX) {
        n = BTL_BOX_COLS_MAX;
    }
    if (n < BTL_BOX_COLS_MIN) {
        n = BTL_BOX_COLS_MIN;
    }
    g_btl_box_cols = n;
    g_btl_box_ox = x;
    g_btl_box_oy = y;
}

void BtlBoxClose(void)
{
    if ((g_btl_box_flags & BTL_BOX_SLIDE) != 0) {
        g_btl_box_step = BTL_BOX_CLOSE;
    } else {
        g_btl_box_step = BTL_BOX_COLLAPSE_STEP;
    }
    if (g_btl_fast_anim != 0) {
        g_btl_box_step = BTL_BOX_CLOSE_NOW;
    }
}

/* The counterpart to BtlOpenMessage, and nothing but a call to BtlBoxClose.
   The cursor code uses it where it would otherwise have opened a help line, so
   the box goes away when the help is turned off. */
void BtlCloseMessage(void)
{
    BtlBoxClose();
}

/* One frame of the box's open or close. Every step ends with the scale at one
   of the two extremes, and the ones that finish drop the step back to zero at
   the shared tail - the ones that are still going return from inside. */
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
