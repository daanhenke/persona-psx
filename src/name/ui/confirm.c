/* Persona 1 (JP) - "is this name right?"  NAME @ 0x800661E0.
 *
 * Everything but the question and its answers dims to a quarter, the two
 * answer boxes appear, and the side caret moves between them. Picking the
 * lower one, or cancelling, brings the screen back as it was.
 */
#include <decomp/types.h>
#include <persona/name/entry.h>

void NameDrawFrame(void);

#define CONFIRM_YES_Y 0x88
#define CONFIRM_NO_Y  0x98

/* Answers 0 for yes; for no the screen is restored first. */
int NameConfirm(void)
{
    int i;

    for (i = 0; i < NAME_SPRITES; i++) {
        if (i != 6 && i != SPR_SIDE_CARET && (i < 23 || i > 25) &&
            (i < 46 || i > 48) && (i < 20 || i > 22) && (i < 39 || i > 41)) {
            g_sprites[i].r = g_sprites[i].g = g_sprites[i].b = 0x20;
        }
    }
    g_bg_keys.r = g_bg_keys.g = g_bg_keys.b = 0x20;
    g_bg_frame.r = g_bg_frame.g = g_bg_frame.b = 0x20;
    g_bg_fields.r = g_bg_fields.g = g_bg_fields.b = 0x20;
    g_sprite_attr[20] &= ~ATTR_HIDE;
    g_sprite_attr[21] &= ~ATTR_HIDE;
    g_sprite_attr[22] &= ~ATTR_HIDE;
    g_sprite_attr[39] &= ~ATTR_HIDE;
    g_sprite_attr[40] &= ~ATTR_HIDE;
    g_sprite_attr[41] &= ~ATTR_HIDE;
    NameDrawFrame();
    g_sprites[SPR_SIDE_CARET].x = 0xC0;
    g_sprites[SPR_SIDE_CARET].y = CONFIRM_NO_Y;
    for (;;) {
        if (g_name_repeat == 0) {
            if ((g_pad_held & PAD_UP) || (g_pad_held & PAD_DOWN)) {
                NamePlaySe(0);
                g_sprites[SPR_SIDE_CARET].y ^= 0x10;
            } else if (g_pad_trig & PAD_OK) {
                break;
            } else if (g_pad_trig & PAD_BACK) {
                NamePlaySe(2);
                g_sprites[SPR_SIDE_CARET].y = CONFIRM_NO_Y;
                break;
            }
        }
        NameDrawFrame();
    }
    if (g_sprites[SPR_SIDE_CARET].y == CONFIRM_YES_Y) {
        return 0;
    }
    for (i = 0; i < NAME_SPRITES; i++) {
        g_sprites[i].r = g_sprites[i].g = g_sprites[i].b = 0x80;
    }
    g_bg_keys.r = g_bg_keys.g = g_bg_keys.b = 0x80;
    g_bg_frame.r = g_bg_frame.g = g_bg_frame.b = 0x80;
    g_bg_fields.r = g_bg_fields.g = g_bg_fields.b = 0x80;
    g_sprite_attr[20] |= ATTR_HIDE;
    g_sprite_attr[21] |= ATTR_HIDE;
    g_sprite_attr[22] |= ATTR_HIDE;
    g_sprite_attr[39] |= ATTR_HIDE;
    g_sprite_attr[40] |= ATTR_HIDE;
    g_sprite_attr[41] |= ATTR_HIDE;
    return 1;
}
