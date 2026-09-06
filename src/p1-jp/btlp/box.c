/* Persona 1 (JP) - the battle's message box.  BTLP only.
 *   0x8007A964 BtlBoxOpen   0x8007AA5C BtlBoxClose
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
#include <persona/btlp/box.h>

extern u_char g_btl_fast_anim;

extern void BtlBoxLoad(void);

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
