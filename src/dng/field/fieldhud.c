/* Persona 1 (JP) - the field's HUD sprites.  DNG only.
 *   0x8006DF30 FieldInitHud
 */
#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <persona/dng/field.h>

/* The HUD's fixed labels, a byte per character: the low five bits are the
   glyph's column in the font page, the top three its row. */
extern u_char g_hud_text_a[4][5];
extern u_char g_hud_text_b[9][4];
extern u_char g_hud_text_c[4];

#define GLYPH_U(c) (((c) & 0x1F) * 8)
#define GLYPH_V(c) (((c) >> 5) * 12)

#define FONT_TPAGE 0x1E

/* Sets up the HUD's sprites: the compass and its four marks, the frames
   around the map and the clock, the moon icon for the current phase, and
   the three blocks of labels, one sprite per character. The wall palettes
   are loaded last. The two label blocks number their sprites through
   block-scoped locals: one pseudo per loop is what lets local-alloc give
   the column and the sum the same register, as the image does. */
void FieldInitHud(void)
{
    int i;
    int j;
    int n;
    u_char unused[8]; /* the image's frame holds 8 bytes nothing touches */

    FieldInitSprite(0, 12, 12, 5, 0xE4, 0, 0x100, 0x1E0);
    g_scene->sprites[0].attribute = 0x40000000;
    g_scene->sprites[0].x = 0x5E;
    g_scene->sprites[0].y = -0x36;
    FieldInitSprite(1, 12, 12, 5, 0, 12, 0x100, 0x1E0);
    g_scene->sprites[1].attribute = 0;
    FieldInitSprite(2, 12, 12, 5, 12, 12, 0x100, 0x1E0);
    g_scene->sprites[2].attribute = 0;
    FieldInitSprite(3, 12, 12, 5, 0x18, 12, 0x100, 0x1E0);
    g_scene->sprites[3].attribute = 0;
    FieldInitSprite(4, 12, 12, 5, 0x24, 12, 0x100, 0x1E0);
    g_scene->sprites[4].attribute = 0;
    FieldInitSprite(0x41, 0x5B, 0x1A, 5, 0, 0xE0, 0, 0x1E0);
    g_scene->sprites[0x41].x = -0x90;
    g_scene->sprites[0x41].y = -0x6C;
    FieldInitSprite(0x42, 0x40, 0xE, 5, 0x40, 0xD0, 0, 0x1E0);
    g_scene->sprites[0x42].x = -0x90;
    g_scene->sprites[0x42].y = -0x52;
    FieldInitSprite(0x43, 6, 0xE, 5, 0x60, 0xE0, 0, 0x1E0);
    g_scene->sprites[0x43].x = -0x50;
    g_scene->sprites[0x43].y = -0x52;
    FieldInitSprite(0x45, 0x18, 0x18, 0x15, 0, 0x90, 0, 0x1FF);
    g_scene->sprites[0x45].x = -0x8D;
    g_scene->sprites[0x45].y = -0x6B;
    FieldInitSprite(0x44, 0x18, 0x18, 0x15, (u_char)(g_moon_cells[MOON_PHASE] % 10) * 0x18,
                    (u_char)(g_moon_cells[MOON_PHASE] / 10) * 0x18 + 0x90, 0, 0x1FF);
    g_scene->sprites[0x44].x = -0x8D;
    g_scene->sprites[0x44].y = -0x6B;
    g_scene->sprites[0x44].attribute |= 0x60000000;

    for (j = 0; j < 4; j++) {
        for (i = 0; i < 5; i++) {
            int col = i + 5;
            int spr = j * 5 + col;

            FieldInitSprite(spr, 8, 12, FONT_TPAGE, GLYPH_U(g_hud_text_a[j][i]),
                            GLYPH_V(g_hud_text_a[j][i]), 0x100, 0x1E0);
            g_scene->sprites[spr].x = i * 8 - 0x75;
            g_scene->sprites[spr].y = -0x51;
            g_scene->sprites[spr].attribute = 0;
        }
    }
    for (j = 0; j < 9; j++) {
        for (i = 0; i < 4; i++) {
            int col = i + 0x19;
            int spr = j * 4 + col;

            FieldInitSprite(spr, 8, 12, FONT_TPAGE, GLYPH_U(g_hud_text_b[j][i]),
                            GLYPH_V(g_hud_text_b[j][i]), 0x100, 0x1E0);
            g_scene->sprites[spr].x = i * 8 - 0x6A;
            g_scene->sprites[spr].y = -0x6A;
            g_scene->sprites[spr].attribute = 0;
        }
    }
    for (j = 0; j < 4; j++) {
        n = j + 0x3D;
        FieldInitSprite(n, 8, 12, FONT_TPAGE, GLYPH_U(g_hud_text_c[j]),
                        GLYPH_V(g_hud_text_c[j]), 0x100, 0x1E0);
        g_scene->sprites[n].x = j * 8 - 0x5A;
        g_scene->sprites[n].y = -0x60;
        g_scene->sprites[n].attribute = 0;
    }
    FieldLoadWallCluts();
}
