/* Persona 1 (JP) - moving the cursor around an effect's grid.  BTLP only.
 *   0x80079B38 BtlEffectMoveCursor
 *
 * Only the effect holding g_btl_effect_cur takes the pad, and only while bit
 * 0x10 of its kind is clear. The grid is one halfword - columns in the low
 * byte, rows in the high one - and the four face buttons move around it:
 * triangle up a row and cross down, both wrapping through the whole grid;
 * circle right and square left, each wrapping inside its own row.
 *
 * The chain is then walked for the row whose number matches where the cursor
 * landed, and that row becomes the effect's step. A number no row answers to
 * leaves both the cursor and the step where they were, which is how a grid
 * with holes in it is handled.
 *
 * Whether the cursor moved or not, the drawer the step's kind names is run and
 * the hardware cursor is put on the row - eight pixels a step from the
 * effect's own origin, less half a cell so it sits beside the text rather than
 * on it.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <persona/btlp/effect.h>
#include <persona/btlp/input.h>
#include <persona/btlp/sound.h>

/* The bank and sound every move makes. */
#define BTL_EFFECT_CLICK_BANK 1
#define BTL_EFFECT_CLICK_SE   0

/* A row's position is in eight-pixel steps, and the cursor sits this far
   before it. */
#define BTL_EFFECT_CELL 8
#define BTL_EFFECT_DX   (-0x10)
#define BTL_EFFECT_DY   (-1)

extern short         g_btl_effect_ox;
extern short         g_btl_effect_oy;

extern void (*g_btl_effect_cursor_fn[])(void);


/* The cursor is placed through an int-typed call: this unit was built
   against a BtlCursorPlace that took ints, so the image passes the two sums
   without narrowing them (input.h keeps the short one, which is the
   definition's). The selection and each step's temporary are shorts, which
   is what keeps the temporary apart from the selection and sign-extends it
   only where the sum can leave a byte. The walk's body opens with a
   declaration, and that block note is what stops gcc rolling its first test
   to the bottom of the loop. The routine is implicitly int - the image leaves
   the delay slot before its exit empty. */
int BtlEffectMoveCursor(int slot)
{
    BtlEffect    *e;
    BtlEffectRow *row;
    BtlEffectRow *step;
    u_short       grid;
    int           cols;
    int           rows;
    short         sel;
    short         t;
    short         keep;

    e = g_btl_effect[slot];
    if (slot == g_btl_effect_cur
        && (step = g_btl_effect_step[slot]) != (BtlEffectRow *)-1) {
        row = step;
        if ((e->kind & BTL_EFFECT_NOPAD) == 0) {
            grid = e->grid;
            keep = e->sel;
            cols = grid & 0xFF;
            rows = grid >> 8;
            sel = keep;
            if ((BtlInputKeys() & g_btl_key_up) != 0) {
                BtlSePlay(BTL_EFFECT_CLICK_BANK, BTL_EFFECT_CLICK_SE);
                t = keep - cols;
                sel = t;
                if (t < 0) {
                    sel = t + cols * rows;
                }
            }
            if ((BtlInputKeys() & g_btl_key_down) != 0) {
                BtlSePlay(BTL_EFFECT_CLICK_BANK, BTL_EFFECT_CLICK_SE);
                t = sel + cols;
                sel = t;
                if (cols * rows <= t) {
                    sel = t - cols * rows;
                }
            }
            if ((BtlInputKeys() & g_btl_key_right) != 0) {
                BtlSePlay(BTL_EFFECT_CLICK_BANK, BTL_EFFECT_CLICK_SE);
                t = sel + 1;
                sel = t;
                if (t % cols == 0) {
                    sel = t - cols;
                }
            }
            if ((BtlInputKeys() & g_btl_key_left) != 0) {
                BtlSePlay(BTL_EFFECT_CLICK_BANK, BTL_EFFECT_CLICK_SE);
                if (sel % cols == 0) {
                    sel = sel + cols - 1;
                } else {
                    sel = sel - 1;
                }
            }

            row = e->next;
            for (;;) {
                BtlEffectRow *next;

                if (row->row == (u_char)sel) {
                    break;
                }
                next = row->next;
                if (next == (BtlEffectRow *)-1) {
                    goto done;
                }
                row = next;
            }
            keep = sel;
            g_btl_effect_step[g_btl_effect_cur] = row;
        done:
            e->sel = keep;
            row = g_btl_effect_step[g_btl_effect_cur];
        }
        g_btl_effect_cursor_fn[row->kind & 0xF]();
        ((void (*)(int, int))BtlCursorPlace)(
            e->curx + g_btl_effect_ox + row->x * BTL_EFFECT_CELL
                + BTL_EFFECT_DX,
            e->cury + g_btl_effect_oy + row->y * BTL_EFFECT_CELL
                + BTL_EFFECT_DY);
    }
}

