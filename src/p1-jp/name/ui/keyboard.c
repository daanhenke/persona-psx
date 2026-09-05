/* Persona 1 (JP) - the on-screen keyboard.  NAME @ 0x80067F20.
 *
 * Four pages of six rows of ten cells. A row is drawn by expanding its ten
 * glyphs into one strip and uploading the strip; the upload is inside the
 * column loop, so the row goes up again after every glyph.
 */
#include <types.h>

#define KEY_PAGES 4
#define KEY_ROWS  6
#define KEY_COLS  10
#define CELL_H    0x10
#define KEY_X     0x200
#define KEY_Y     0x30

extern u_short g_keyboard[KEY_PAGES][KEY_ROWS][KEY_COLS];
extern u_long  g_glyph_cell[];

extern void ExpandGlyph(u_short code, u_long *dst, int stride);
extern void UploadImage(int x, int y, int w, int h, u_long *data);

void NameDrawKeyboardPage(int page)
{
    int row;
    int col;

    if (page >= KEY_PAGES) {
        return;
    }
    for (row = 0; row < KEY_ROWS; row++) {
        for (col = 0; col < KEY_COLS; col++) {
            ExpandGlyph(g_keyboard[page][row][col], &g_glyph_cell[col * 2],
                        KEY_COLS * 2);
            UploadImage(KEY_X, row * CELL_H + KEY_Y, KEY_COLS * 4, CELL_H,
                        g_glyph_cell);
        }
    }
}
