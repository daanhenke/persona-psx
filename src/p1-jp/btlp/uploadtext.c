/* Persona 1 (JP) - a run of text into VRAM.  BTLP only.
 *   0x8007B4AC BtlUploadText
 *
 * The contact box's four cells each carry a line of text and a place to put it.
 * This walks one line a character at a time: expand the glyph into the staging
 * area, queue the staging cell as a small upload, and step four pixels along.
 *
 * It stops at either terminator or at fifteen characters, whichever comes
 * first, and refuses to start at all on an empty line. The check inside the
 * loop is on the second byte of the pair, because a value is two bytes and a
 * field ends between them.
 */
#include <types.h>
#include <persona/btlp/window.h>

/* Ends a run; 0xF5 also separates one field from the next. */
#define STR_END   0xFF
#define STR_FIELD 0xF5

#define BTL_TEXT_MAX 15

/* Where the contact box's lines are staged in VRAM. */
#define BTL_TEXT_X 0x380
#define BTL_TEXT_Y 0x150

/* One of the contact box's cells: its text and where it goes. */
typedef struct {
    /* 0x0 */ u_long text;
    /* 0x4 */ short  x;
    /* 0x6 */ short  y;
} BtlTalkCell;                  /* 8 bytes */

/* The font bitmaps, passed by address as a plain value. */
#define g_font_bits 0x801E0000

extern const u_char *BtlSeqReadValue(const u_char *p, u_short *out);
extern void BtlExpandGlyph(int code, u_int *dest, int font);

/* No prototype: this file hands it plain ints and lets the callee narrow them. */
extern void BtlQueueVramLoad();

void BtlUploadText(const BtlTalkCell *cell, const u_char *text)
{
    u_short value[4];
    int     n;
    int     x;

    n = 0;
    if (*text != STR_END && *text != STR_FIELD) {
        x = BTL_TEXT_X;
        while (n < BTL_TEXT_MAX && text[1] != STR_FIELD) {
            text = BtlSeqReadValue(text, value);
            BtlExpandGlyph(value[0],
                           (u_int *)&g_btl_glyph_cells[g_btl_glyph_next *
                                                       BTL_GLYPH_STRIDE],
                           g_font_bits);
            BtlQueueVramLoad(&g_btl_glyph_cells[g_btl_glyph_next *
                                                BTL_GLYPH_STRIDE],
                             cell->x * 2 + x, cell->y + BTL_TEXT_Y,
                             BTL_GLYPH_W, BTL_GLYPH_H);
            g_btl_glyph_next++;
            n++;
            x += BTL_GLYPH_W;
            if (*text == STR_END) {
                break;
            }
            if (*text == STR_FIELD) {
                break;
            }
        }
    }
}
