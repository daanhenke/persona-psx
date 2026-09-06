/* Persona 1 (JP) - one character into a message window.  BTLP only.
 *   0x8007BC9C BtlWindowPutChar
 *
 * Three steps: read the next value out of the script, expand that glyph into
 * the staging area, and queue the staging cell as a small VRAM upload. The
 * character then gets a place in the window's own cell array, and both of the
 * window's counters move on - the one that says how many characters are on
 * screen and the one that says which staging cell is in use.
 *
 * Both counters wrap the same way, fifteen characters to a row, but they are
 * kept apart because the staging cells are reused while the characters on
 * screen are not.
 */
#include <types.h>
#include <persona/btlp/window.h>

/* The font bitmaps, passed by address as a plain value. */
#define g_font_bits 0x801E0000

extern const u_char *BtlSeqReadValue(const u_char *p, u_short *out);
extern void BtlExpandGlyph(int code, u_int *dest, int font);
/* No prototype: this file hands it plain ints and lets the callee narrow them. */
extern void BtlQueueVramLoad();

const u_char *BtlWindowPutChar(BtlWindow *w, const u_char *script)
{
    BtlWindowCell *cell;
    const u_char  *next;
    u_short        value[4];

    cell = &w->cells[w->staged];
    next = BtlSeqReadValue(script, value);
    BtlExpandGlyph(value[0],
                   (u_int *)&g_btl_glyph_cells[g_btl_glyph_next * BTL_GLYPH_STRIDE],
                   g_font_bits);
    BtlQueueVramLoad((u_long *)&g_btl_glyph_cells[g_btl_glyph_next * BTL_GLYPH_STRIDE],
                     w->vram_x + (w->staged % BTL_GLYPH_ROW) * BTL_GLYPH_W,
                     w->vram_y + (w->staged / BTL_GLYPH_ROW) * BTL_GLYPH_H,
                     BTL_GLYPH_W, BTL_GLYPH_H);
    cell->x = (w->placed - (w->placed / BTL_GLYPH_ROW) * BTL_GLYPH_ROW) * BTL_GLYPH_H;
    cell->y = (w->placed / BTL_GLYPH_ROW) * BTL_GLYPH_H;
    g_btl_glyph_next++;
    cell->attr = w->attr;
    w->placed++;
    w->staged++;
    return next;
}
