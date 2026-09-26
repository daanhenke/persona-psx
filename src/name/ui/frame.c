/* Persona 1 (JP) - one frame of the name screen, and the pad.
 * NAME @ 0x8006755C.
 */
#include <decomp/types.h>
#include <libetc.h>
#include <persona/name/entry.h>

/* Sorts the backgrounds and every shown sprite, waits out the frame, pulses
   the caret colour, reads the pad and draws. */
void NameDrawFrame(void)
{
    int     buf;
    int     i;
    u_long  clut;

    buf = GsGetActiveBuff();
    GsSetWorkBase(g_packets[buf]);
    GsClearOt(0, 0, &g_ot[buf]);
    if (g_name_mode == 1) {
        GsSortFastBg(&g_bg_frame, &g_ot[buf], 1);
        GsSortFastBg(&g_bg_fields, &g_ot[buf], 1);
    }
    GsSortFastBg(&g_bg_list, &g_ot[buf], 1);
    GsSortFastBg(&g_bg_keys, &g_ot[buf], 1);
    if ((g_blink_tick & 7) == 0) {
        g_sprites[SPR_PREVIEW].attribute ^= 0x80000000;
    }
    for (i = 0; i < NAME_SPRITES; i++) {
        if (!(g_sprite_attr[i] & ATTR_HIDE)) {
            if (g_sprite_attr[i] & 0x40) {
                GsSortSprite(&g_sprites[i], &g_ot[buf], g_sprite_attr[i] & 0xF);
            } else {
                GsSortFastSprite(&g_sprites[i], &g_ot[buf], g_sprite_attr[i] & 0xF);
            }
        }
    }
    DrawSync(0);
    VSync(0);
    g_blink_tick++;
    if ((g_name_blink == 1) | (g_name_blink == 0x1F)) {
        g_name_blink_step ^= 0xFE;
    }
    g_name_blink += g_name_blink_step;
    clut = (g_name_blink << 5) | (g_name_blink << 10);
    UploadImage(0x10F, 0x1E2, 1, 1, &clut);
    NamePadRead();
    GsSwapDispBuff();
    GsSortClear(0, 0, 0, &g_ot[buf]);
    GsDrawOt(&g_ot[buf]);
}

/* Turns the pad into presses: a new button counts at once, a held one first
   after 25 frames and then every five. */
void NamePadRead(void)
{
    g_pad_prev = g_pad_raw;
    g_pad_raw = PadRead(1);
    if (g_pad_raw != 0) {
        if ((g_pad_trig = (g_pad_raw & g_pad_prev) ^ g_pad_raw) == 0) {
            if (g_name_first) {
                if (g_name_repeat >= 25) {
                    g_pad_held = g_pad_raw;
                    g_name_first = 0;
                } else {
                    g_name_repeat++;
                }
            } else if (g_name_repeat < 5) {
                g_name_repeat++;
            } else {
                g_pad_held = g_pad_raw;
                g_name_repeat = 0;
            }
        } else {
            g_pad_held = g_pad_trig;
            g_name_repeat = 0;
            g_name_first = 1;
        }
    } else {
        g_pad_held = 0;
        g_name_repeat = 0;
        g_name_first = 0;
    }
}
