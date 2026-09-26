/* Persona 1 (JP) - the name screen's carets.  NAME @ 0x8006652C.
 */
#include <decomp/types.h>
#include <persona/name/entry.h>

/* Puts the key caret on the key under the cursor - the end buttons have a
   box of their own - and the text caret under the cell being typed into. */
void NameCaretPlace(void)
{
    if (g_name_col == 0 || g_name_col == KEY_COLS + 1) {
        g_sprites[SPR_SIDE_CARET].x = g_caret_x;
        g_sprites[SPR_SIDE_CARET].y = g_caret_y;
        g_sprite_attr[SPR_SIDE_CARET] &= ~ATTR_HIDE;
        g_sprite_attr[SPR_KEY_CARET] |= ATTR_HIDE;
    } else {
        g_sprites[SPR_KEY_CARET].x = g_caret_x + 8;
        g_sprites[SPR_KEY_CARET].y = g_caret_y + 8;
        g_sprite_attr[SPR_KEY_CARET] &= ~ATTR_HIDE;
        g_sprite_attr[SPR_SIDE_CARET] |= ATTR_HIDE;
    }
    g_sprites[SPR_TEXT_CARET].x = g_name_cursor[g_name_field] * 16 + 0xA0;
    g_sprites[SPR_TEXT_CARET].y = g_name_field * 16 + 0x28;
}

/* Shows the kanji the list is on in the preview box, or hides the box. */
void NameListPreview(void)
{
    if (g_list_rows[g_list_top][1] != 0) {
        g_sprite_attr[SPR_PREVIEW] &= ~ATTR_HIDE;
        ExpandGlyph(g_list_code + 1, g_glyph_cell, 2);
        UploadImage(0x238, 0x20, 4, 0x10, g_glyph_cell);
    } else {
        g_sprite_attr[SPR_PREVIEW] |= ATTR_HIDE;
    }
}

/* Leaves the kanji list for the keyboard. */
void NameListClose(void)
{
    g_name_mode = 0;
    g_sprite_attr[44] &= ~ATTR_HIDE;
    g_sprite_attr[45] &= ~ATTR_HIDE;
    g_sprite_attr[10] &= ~ATTR_HIDE;
    g_sprite_attr[11] |= ATTR_HIDE;
    g_sprite_attr[SPR_PREVIEW] |= ATTR_HIDE;
}
