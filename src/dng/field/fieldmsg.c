/* Persona 1 (JP) - the field's message box.  DNG only.
 *   0x80074AB8 FieldMsgPutGlyph
 *   0x80074BC0 FieldMsgClear
 *   0x80074C3C FieldMsgNewLine
 *   0x80074D64 FieldMsgClearLine
 *   0x80074DD8 FieldMsgSetWindow
 *   0x80074EB4 FieldMsgTint
 *
 * Text is drawn into a strip of VRAM beside the screen, three lines of
 * fifteen glyphs, and shown through a primitive list in the pack whose cells
 * carry the text colour's palette. A fourth line scrolls the window up.
 */
#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <persona/dng/field.h>

#define MSG_LINES   3
#define MSG_COLS    15
#define MSG_VRAM_X  320
#define MSG_VRAM_Y  24
#define MSG_LINE_H  16

/* The message box's cells in the pack. */
#define MSG_CELLS ((u_char *)PACK_BASE + g_pack_msg_tab[3])

/* Draws glyph `n` at the cursor and moves it on, starting a new line when
   the one it is on is full. */
void FieldMsgPutGlyph(u_short n)
{
    if (g_msg_col >= MSG_COLS) {
        FieldMsgNewLine();
    }
    FieldDecodeGlyph(n);
    FieldMsgTint(MSG_CELLS, g_msg_color, g_msg_col, g_msg_line, 1);
    g_msg_rect.x = g_msg_col * 4 + MSG_VRAM_X;
    g_msg_rect.y = g_msg_line % MSG_LINES * MSG_LINE_H + MSG_VRAM_Y;
    LoadImage(&g_msg_rect, (u_long *)g_scene->glyph);
    g_msg_col++;
}

/* Empties the box and recolours all of it. */
void FieldMsgClear(void)
{
    RECT r;

    r.x = MSG_VRAM_X;
    r.y = MSG_VRAM_Y;
    r.w = MSG_COLS * 4;
    r.h = MSG_LINES * MSG_LINE_H;
    ClearImage(&r, 0, 0, 0);
    FieldMsgTint(MSG_CELLS, g_msg_color, 0, 0, MSG_LINES * MSG_COLS);
}

/* Moves the cursor to the start of the next line. From the fourth line on
   the window scrolls up a line over four frames and the line it reuses is
   wiped. */
void FieldMsgNewLine(void)
{
    int i;

    g_msg_col = 0;
    if (++g_msg_line >= MSG_LINES) {
        for (i = 0; i < 4; i++) {
            g_scene->msg_scroll_a -= 4;
            func_80065978();
        }
        FieldMsgClearLine(g_msg_line % MSG_LINES);
        g_scene->msg_scroll_a += MSG_LINE_H;
        g_scene->msg_scroll_b += MSG_LINE_H;
    }
    FieldMsgTint(MSG_CELLS, g_msg_color, 0, g_msg_line, MSG_COLS);
}

/* Wipes one line of the box's VRAM. */
void FieldMsgClearLine(int line)
{
    RECT r;

    r.x = MSG_VRAM_X;
    r.y = line % MSG_LINES * MSG_LINE_H + MSG_VRAM_Y;
    r.w = MSG_COLS * 4;
    r.h = MSG_LINE_H;
    ClearImage(&r, 0, 0, 0);
}

/* Sets the selection window up: the default kind when `n` is 0, otherwise
   kind n's entry of the window table. */
void FieldMsgSetWindow(int n)
{
    g_scene->win.y = 0x1E0;
    g_scene->win.rows = 1;
    if (n == 0) {
        g_scene->win.kind = 2;
        g_scene->win.first = 0x20;
        g_scene->win.count = 11;
        g_scene->win.src = g_win_default;
    } else {
        g_scene->win.kind = 1;
        g_scene->win.first = n + 0x1F;
        g_scene->win.count = 1;
        g_scene->win.src = &g_win_kinds[n];
    }
}

/* Gives `count` cells from (col, line) the palette of text colour `color`. */
void FieldMsgTint(u_char *cells, u_char color, int col, int line, int count)
{
    cells += ((line % MSG_LINES) * MSG_COLS + col) * 8 + 0xA;
    for (; count > 0; count--) {
        *(u_short *)cells = g_msg_cluts[color];
        cells += 8;
    }
}
