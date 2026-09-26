/* Persona 1 (JP) - keyboard pages and the typing cursor.  NAME @ 0x80066F68.
 */
#include <decomp/types.h>
#include <persona/name/entry.h>

/* Swaps keyboard pages: the old page's two tabs drop back to normal size,
   the new page's grow, and the keys are redrawn. */
void NameSetPage(u_char page)
{
    g_sprites[g_name_page + SPR_PAGE_TAB].scalex = 0x1000;
    g_sprites[g_name_page + SPR_PAGE_TAB].scaley = 0x1000;
    g_sprites[g_name_page + SPR_PAGE_TAB2].scalex = 0x1000;
    g_sprites[g_name_page + SPR_PAGE_TAB2].scaley = 0x1000;
    g_name_page = page;
    g_sprites[g_name_page + SPR_PAGE_TAB].scalex = 0x1400;
    g_sprites[g_name_page + SPR_PAGE_TAB].scaley = 0x1200;
    g_sprites[g_name_page + SPR_PAGE_TAB2].scalex = 0x1200;
    g_sprites[g_name_page + SPR_PAGE_TAB2].scaley = 0x1200;
    NameDrawKeyboardPage(g_name_page);
}

/* Redraws the cell the caret sits on in the field being edited. */
void NameDrawCursor(void)
{
    ExpandGlyph(g_name_text[g_name_field][g_name_cursor[g_name_field]],
                g_glyph_cell, 2);
    UploadImage(g_name_x[g_name_field] + 0x200 +
                    g_name_cursor[g_name_field] * 4,
                g_name_y[g_name_field], 4, 0x10, g_glyph_cell);
}
