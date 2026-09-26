/* Persona 1 (JP) - the kanji list.  NAME @ 0x80066758.
 *
 * Rows of ten kanji, grouped under the kana they read as. The list keeps its
 * rows in a ring of seven strips of VRAM and scrolls the background over
 * them, so a step draws only the one row coming into view; g_list_rows says
 * which group and row each strip holds. A group's first row is drawn with its
 * kana in front, the rest with a blank.
 */
#include <decomp/types.h>
#include <persona/name/entry.h>

/* Draws the row before the top one (`back`) or after the bottom one into the
   strip that is about to come into view. */
void NameListScroll(u_char back)
{
    int group;
    int row;
    int slot;
    int k;

    slot = (g_list_top + 6) % LIST_SLOTS;
    if (back) {
        row = g_list_rows[g_list_top][1];
        group = g_list_rows[g_list_top][0];
        if (--row < 0) {
            if (--group < 0) {
                group = KANJI_GROUPS - 1;
            } else if (g_kanji_row_counts[group] == 0) {
                group--;
            }
            row = g_kanji_row_counts[group] - 1;
        }
    } else {
        group = g_list_rows[(g_list_top + 5) % LIST_SLOTS][0];
        row = g_list_rows[(g_list_top + 5) % LIST_SLOTS][1];
        if (++row >= g_kanji_row_counts[group]) {
            if (++group >= KANJI_GROUPS) {
                group = 0;
            } else if (g_kanji_row_counts[group] == 0) {
                group++;
            }
            row = 0;
        }
    }
    if (row == 0) {
        ExpandGlyph(group + 1, g_glyph_cell, 0x16);
    } else {
        ExpandGlyph(0, g_glyph_cell, 0x16);
    }
    for (k = 0; k < KANJI_COLS; k++) {
        ExpandGlyph(g_kanji_rows[group][row][k], &g_glyph_cell[(k + 1) * 2], 0x16);
    }
    UploadImage(0x200, slot * 16 + 0x30, 0x2C, 0x10, g_glyph_cell);
    g_list_rows[slot][0] = group;
    g_list_rows[slot][1] = row;
}

/* Fills all six strips from `row` of `group` on. */
void NameListFill(u_char group, u_char row)
{
    int i;
    int k;

    g_bg_frame.scrolly = 0;
    g_bg_fields.scrolly = 0;
    for (i = 0; i < LIST_ROWS; i++) {
        if (g_kanji_row_counts[group] != 0) {
            if (row == 0) {
        ExpandGlyph(group + 1, g_glyph_cell, 0x16);
    } else {
        ExpandGlyph(0, g_glyph_cell, 0x16);
    }
            for (k = 0; k < KANJI_COLS; k++) {
                ExpandGlyph(g_kanji_rows[group][row][k], &g_glyph_cell[(k + 1) * 2], 0x16);
            }
            UploadImage(0x200, i * 16 + 0x30, 0x2C, 0x10, g_glyph_cell);
        }
        g_list_rows[i][1] = row;
        g_list_rows[i][0] = group;
        if (++row >= g_kanji_row_counts[group]) {
            row = 0;
            if (++group >= KANJI_GROUPS) {
                group = 0;
            } else if (g_kanji_row_counts[group] == 0) {
                group++;
            }
        }
    }
}
