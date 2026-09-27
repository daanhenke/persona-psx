/* cc1flags: -O0 -G8 */
/* Persona 1 (JP) - OPEN.EXE, the title @ 0x80081018
 *
 * The title and its demo screens: sprite set-up, the font, the fades and
 * the draw loops. Built without optimisation like the rest of this
 * executable.
 */
#include <decomp/types.h>
#include <libetc.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <libsnd.h>
#include <persona/open/open.h>

/* Defined here, and so reached gp-relative: the pad this frame, last frame
   and what went down (all bits up to start with), the title's cursor, and
   how many font and message cells are out. */
int g_pad_now = -1;
int g_pad_old = 0;
int g_pad_trig = 0;
int g_title_cursor = 0;
int g_text_len = 0;
int g_msg_len = 0;

/* The messages, as font cells: 16x16 glyph numbers. */
u_short g_txt_load_header[24] = {
    0x0127, 0x01DB, 0x000D, 0x0029, 0x0073, 0x0074, 0x0079, 0x00CC,
    0x0057, 0x00CC, 0x008F, 0x0019, 0x005E, 0x007C, 0x00A1, 0x0065,
    0x002D, 0x01DE, 0x002E, 0x003C, 0x0008, 0x0039, 0x000B, 0x0002,
};
u_short g_txt_no_card[14] = {
    0x0073, 0x0074, 0x0079, 0x00CC, 0x0057, 0x00CC, 0x008F, 0x002F,
    0x0001, 0x0028, 0x001F, 0x000E, 0x002E, 0x0000,
};
u_short g_txt_card_error[12] = {
    0x0073, 0x0074, 0x0079, 0x00CC, 0x0057, 0x00CC, 0x008F, 0x002F,
    0x0293, 0x01BB, 0x003C, 0x000D,
};
u_short g_txt_unformatted[26] = {
    0x0000, 0x0000, 0x0073, 0x0074, 0x0079, 0x00CC, 0x0057, 0x00CC,
    0x008F, 0x002F, 0x0000, 0x0000, 0x0000, 0x006D, 0x009E, 0x00CC,
    0x0070, 0x00A1, 0x0065, 0x000B, 0x002A, 0x0013, 0x0002, 0x001F,
    0x000E, 0x002E,
};
u_short g_txt_loading[36] = {
    0x008E, 0x00CC, 0x0061, 0x002D, 0x03DE, 0x0020, 0x022D, 0x002E,
    0x003C, 0x0002, 0x001F, 0x000D, 0x0000, 0x0000, 0x0073, 0x0074,
    0x0079, 0x00CC, 0x0057, 0x00CC, 0x008F, 0x002D, 0x0000, 0x0000,
    0x0000, 0x04AA, 0x0006, 0x0015, 0x0002, 0x003C, 0x0008, 0x0039,
    0x000B, 0x0002, 0x0000, 0x0000,
};
u_short g_txt_save_broken[24] = {
    0x0000, 0x006D, 0x009A, 0x0053, 0x007A, 0x00C0, 0x0019, 0x008E,
    0x00CC, 0x0061, 0x001A, 0x0000, 0x0000, 0x0000, 0x0000, 0x0137,
    0x002A, 0x0013, 0x0002, 0x001F, 0x000D, 0x0000, 0x0000, 0x0000,
};
u_short g_txt_no_saves[12] = {
    0x005F, 0x00CC, 0x0092, 0x008E, 0x00CC, 0x0061, 0x002F, 0x0001,
    0x0028, 0x001F, 0x000E, 0x002E,
};
u_short g_txt_no_cards[22] = {
    0x0073, 0x0074, 0x0079, 0x00CC, 0x0057, 0x00CC, 0x008F, 0x002F,
    0x005F, 0x00A1, 0x0065, 0x0000, 0x0000, 0x000B, 0x002A, 0x0013,
    0x0002, 0x001F, 0x000E, 0x002E, 0x0000, 0x0000,
};
u_short g_txt_card0_full[20] = {
    0x005E, 0x007C, 0x00A1, 0x0065, 0x00C1, 0x0019, 0x02A8, 0x0007,
    0x03F5, 0x01D0, 0x0000, 0x0000, 0x002F, 0x02EE, 0x0028, 0x001F,
    0x000E, 0x002E, 0x0000, 0x0000,
};
u_short g_txt_card1_full[20] = {
    0x005E, 0x007C, 0x00A1, 0x0065, 0x00C2, 0x0019, 0x02A8, 0x0007,
    0x03F5, 0x01D0, 0x0000, 0x0000, 0x002F, 0x02EE, 0x0028, 0x001F,
    0x000E, 0x002E, 0x0000, 0x0000,
};
u_short g_txt_suspend_erase[39] = {
    0x000A, 0x0019, 0x008E, 0x00CC, 0x0061, 0x001A, 0x015F, 0x02FF,
    0x008E, 0x00CC, 0x0061, 0x003C, 0x000D, 0x008E, 0x00CC, 0x0061,
    0x001A, 0x03DE, 0x0020, 0x022D, 0x001F, 0x002A, 0x0010, 0x0150,
    0x0000, 0x0000, 0x0057, 0x00CC, 0x008F, 0x0006, 0x0027, 0x0224,
    0x01C1, 0x000B, 0x002A, 0x001F, 0x000D, 0x0000, 0x0000,
};

/* The font's sprites run 34 to a text row. */
#define FONT_CELL(row, col) ((row) * 34 + (col))

/* The save list's rows are 24 pixels apart from y 0x24. */
#define SAVE_ROW_Y(row) ((short)((row) * 24 + 0x24))

int OpenTitle(int movie)
{
    RECT rect;
    int  i;
    int  j;
    int  state = 0;
    int  n_opts;
    int  avail[3];
    int  res[2];

    ResetCallback();
    SsEnd();
    SsQuit();
    SsInit();
    InitPAD(g_pad_buf0, 4, g_pad_buf1, 4);
    StartPAD();
    if (movie == 1) {
        setRECT(&rect, 0, 0, 0x200, 0x200);
        ClearImage(&rect, 0, 0, 0);
    }
    DrawSync(0);
    ResetGraph(3);
    if (movie == 2) {
        GsInitGraph(320, 240, 4, 0, 0);
    } else {
        GsInitGraph2(320, 240, 4, 0, 0);
    }
    GsDefDispBuff(0, 0, 0, 240);
    OpenLoadTim((u_long *)(OPEN_PACK[0] + 0x80180000), 1);
    DrawSync(0);
    OpenLoadTim((u_long *)(OPEN_PACK[1] + 0x80180000), 0);
    DrawSync(0);
    OpenLoadTim((u_long *)(OPEN_PACK[2] + 0x80180000), 0);
    DrawSync(0);
    OpenLoadTim((u_long *)(OPEN_PACK[3] + 0x80180000), 0);
    DrawSync(0);
    OpenLoadTim((u_long *)(OPEN_PACK[4] + 0x80180000), 0);
    DrawSync(0);
    for (i = 0; i < 10; i++) {
        g_sprite_flags[i] = 0;
    }
    g_ot[1].length = 4;
    g_ot[0].length = 4;
    g_ot[0].org = g_ot_tags[0];
    g_ot[1].org = g_ot_tags[1];
    OpenSpriteInit(9, 0x100, 0xF0, 8, 0, 0, 0, 0);
    g_sprites[9].attribute = 0x2000000;
    g_sprites[9].x = 0;
    g_sprites[9].y = 0;
    g_sprites[9].r = g_sprites[9].g = g_sprites[9].b = 0x80;
    g_sprite_flags[9] = 0x82;
    OpenSpriteInit(8, 0x40, 0xF0, 0xC, 0, 0, 0, 0);
    g_sprites[8].attribute = 0x2000000;
    g_sprites[8].x = 0x100;
    g_sprites[8].y = 0;
    g_sprites[8].r = g_sprites[8].g = g_sprites[8].b = 0xFF;
    g_sprite_flags[8] = 0x82;
    OpenSpriteInit(7, 0x88, 0x10, 0xD, 0, 0, 0, 0x1E0);
    g_sprites[7].x = 0x5F;
    g_sprites[7].y = 0xA4;
    OpenSpriteInit(6, 0x68, 0x10, 0xD, 0, 0x10, 0, 0x1E0);
    g_sprites[6].x = 0x6B;
    g_sprites[6].y = 0x90;
    OpenSpriteInit(5, 0x68, 0x10, 0xD, 0, 0x20, 0, 0x1E0);
    g_sprites[5].x = 0x6B;
    g_sprites[5].y = 0xA0;
    OpenSpriteInit(4, 0x68, 0x10, 0xD, 0, 0x30, 0, 0x1E0);
    g_sprites[4].x = 0x6B;
    g_sprites[4].y = 0xB0;
    OpenSpriteInit(3, 0xB0, 0x28, 0xD, 0, 0x40, 0, 0x1E0);
    g_sprites[3].x = 0x46;
    g_sprites[3].y = 0x92;
    OpenSpriteInit(2, 0x88, 0x10, 0xD, 0, 0x68, 0, 0x1E0);
    g_sprites[2].attribute |= 0x50000000;
    g_sprites[2].x = 0x5B;
    g_sprites[2].y = 0x90;
    g_sprites[0].r = 0x80;
    g_title_idle = 900;
    g_title_cursor = 0;
    if (movie == 0) {
        g_msg_box.attribute = 0x50000000;
        g_msg_box.x = 0;
        g_msg_box.y = 0;
        g_msg_box.w = 0x140;
        g_msg_box.h = 0xF0;
        g_msg_box.r = 0;
        g_msg_box.g = 0;
        g_msg_box.b = 0;
        g_msg_box_shown = 1;
        for (i = 0; i < 0x100; i += 4) {
            g_msg_box.r = g_msg_box.g = g_msg_box.b = 0xFF - i;
            OpenDrawTitle();
            OpenDrawTitle();
        }
    } else {
        g_sprites[9].r = g_sprites[9].g = g_sprites[9].b = 0x80;
        g_sprites[8].r = g_sprites[8].g = g_sprites[8].b = 0x80;
    }
    g_msg_box_shown = 0;
    OpenDrawTitle();
    SsSetStereo();
    SsUtSetReverbType(4);
    SsUtReverbOn();
    SsSetTickMode(1);
    SsSetTableSize(g_seq_table, 4, 1);
    g_open_vab = SsVabOpenHead((u_char *)(OPEN_PACK[5] + 0x80180000), -1);
    SsVabTransBody((u_char *)(OPEN_PACK[6] + 0x80180000), g_open_vab);
    SsVabTransCompleted(1);
    for (i = 0; i < 4; i++) {
        g_open_seq[i] = SsSeqOpen((u_long *)(OPEN_PACK[7 + i] + 0x80180000), g_open_vab);
        SsSeqSetVol(g_open_seq[i], 0x7F, 0x7F);
    }
    SsStart();
    SsSetMVol(0x7F, 0x7F);
    VSync(60);
    SsUtSetReverbDepth(0x40, 0x40);
    g_sprite_flags[7] = 0x80;
    g_pad_now = -1;
    g_pad_trig = 0;
    SetDispMask(1);
    while (--g_title_idle) {
        switch (state) {
        case 0:
            if (g_pad_trig) {
                if (g_pad_trig != 1) {
                    OpenPlaySeq(1);
                } else {
                    OpenPlaySeq(2);
                }
                g_title_idle = 900;
                state = 1;
                g_sprite_flags[7] = 0;
            } else if (g_title_idle % 16 == 0) {
                g_sprite_flags[7] ^= 0x80;
            }
            break;
        case 1:
            g_title_idle = 900;
            g_sprite_flags[7] = 0;
            g_sprite_flags[2] = 0;
            g_sprite_flags[3] = 0x80;
            g_sprite_flags[4] = 0;
            g_sprite_flags[6] = 0;
            OpenDrawTitle();
            OpenDrawTitle();
            avail[0] = 0;
            avail[1] = 1;
            avail[2] = 0;
            n_opts = 1;
            g_title_cursor = 1;
            g_card0_suspends = g_card1_suspends = 0;
            g_card0_saves = g_card1_saves = 0;
            VSync(0);
            res[0] = CardLoad(0);
            VSync(0);
            res[1] = CardLoad(1);
            if (res[0] == 0 || res[1] == 0) {
                VSync(0);
                g_card_status = CardScanSaves(0, 0, g_save_list);
                VSync(0);
                g_load_status = CardScanSaves(1, 0, g_save_list);
                g_card_status = g_card_status == -1 ? 0 : g_card_status;
                g_load_status = g_load_status == -1 ? 0 : g_load_status;
                g_card0_saves = (g_card_status & 0x7F) ^ ((g_card_status & 0x7F00) >> 8);
                g_card1_saves = (g_load_status & 0x7F) ^ ((g_load_status & 0x7F00) >> 8);
                if (g_card0_saves | g_card1_saves) {
                    avail[0] = 1;
                    n_opts++;
                    g_title_cursor = 0;
                }
                VSync(0);
                g_card_status = CardScanSaves(0, 1, g_save_list);
                VSync(0);
                g_load_status = CardScanSaves(1, 1, g_save_list);
                g_card_status = g_card_status == -1 ? 0 : g_card_status;
                g_load_status = g_load_status == -1 ? 0 : g_load_status;
                g_card0_suspends = (g_card_status & 0x7F) ^ ((g_card_status & 0x7F00) >> 8);
                g_card1_suspends = (g_load_status & 0x7F) ^ ((g_load_status & 0x7F00) >> 8);
                if (g_card0_suspends | g_card1_suspends) {
                    avail[2] = 1;
                    n_opts++;
                    g_title_cursor = 2;
                }
            }
            if (avail[0]) {
                g_sprite_flags[6] = 0x80;
            } else {
                g_sprite_flags[6] = 0;
            }
            if (avail[2]) {
                g_sprite_flags[4] = 0x80;
            } else {
                g_sprite_flags[4] = 0;
            }
            g_sprite_flags[5] = 0x80;
            g_sprite_flags[3] = 0;
            g_sprite_flags[2] = 0x81;
            g_sprites[2].y = g_title_cursor * 16 + 0x90;
            state = 2;
            break;
        case 2:
            if (g_pad_trig) {
                g_title_idle = 900;
                if (n_opts != 1) {
                    if (g_pad_trig & 0x1000) {
                        OpenPlaySeq(0);
                        g_title_cursor--;
                        while (1) {
                            g_title_cursor = g_title_cursor < 0 ? 2 : g_title_cursor;
                            if (avail[g_title_cursor] == 0) {
                                g_title_cursor--;
                            } else {
                                break;
                            }
                        }
                    }
                    if ((g_pad_trig & 0x4000) || (g_pad_trig & 0x100)) {
                        OpenPlaySeq(0);
                        g_title_cursor++;
                        while (1) {
                            g_title_cursor = g_title_cursor < 3 ? g_title_cursor : 0;
                            if (avail[g_title_cursor] == 0) {
                                g_title_cursor++;
                            } else {
                                break;
                            }
                        }
                    }
                    g_sprites[2].y = g_title_cursor * 16 + 0x90;
                }
                if (g_pad_trig & 0x820) {
                    g_card_status = 0;
                    switch (g_title_cursor) {
                    case 0:
                        OpenPlaySeq(1);
                        if (OpenLoadMenu(g_title_choice / 2) == 0) {
                            g_card_scan = -1;
                            goto out;
                        }
                        state = 1;
                        break;
                    case 1:
                        if (res[0] == 2 && res[1] == 2) {
                            OpenMessageOpen(11, 2, 0x48, 0x5C, g_txt_no_cards);
                            for (;;) {
                                OpenDrawTitle();
                                if (g_pad_trig) {
                                    break;
                                }
                            }
                            g_msg_len = 0;
                        } else if ((res[0] == 0 || res[1] == 0) && g_card0_saves == 0 && g_card1_saves == 0) {
                            g_card0_saves = g_card1_saves = 0;
                            if (res[0] == 0) {
                                VSync(0);
                                if ((g_card0_saves = CardCheckFree(0)) == -1) {
                                    OpenMessageOpen(10, 2, 0x50, 0x5C, g_txt_card0_full);
                                    for (;;) {
                                        OpenDrawTitle();
                                        if (g_pad_trig) {
                                            break;
                                        }
                                    }
                                    g_msg_len = 0;
                                }
                            }
                            if (res[1] == 0) {
                                VSync(0);
                                if ((g_card1_saves = CardCheckFree(1)) == -1) {
                                    OpenMessageOpen(10, 2, 0x50, 0x5C, g_txt_card1_full);
                                    for (;;) {
                                        OpenDrawTitle();
                                        if (g_pad_trig) {
                                            break;
                                        }
                                    }
                                    g_msg_len = 0;
                                }
                            }
                            if (g_card0_saves != -1 && g_card1_saves != -1) {
                                OpenPlaySeq(1);
                            }
                        } else {
                            OpenPlaySeq(1);
                        }
                        bzero((u_char *)0x801F1BCC, 0x3780);
                        for (i = 0x80; i >= 0; i -= 4) {
                            g_sprites[0].r = g_sprites[0].g = g_sprites[0].b = i;
                            OpenDrawTitle();
                        }
                        goto chosen;
                    case 2:
                        OpenPlaySeq(1);
                        if (OpenLoadMenu(1) == 0) {
                            goto chosen;
                        }
                        state = 1;
                        break;
                    }
                } else if (g_pad_trig & 0x40) {
                    OpenPlaySeq(2);
                    state = 1;
                }
            }
            break;
        }
        if (state >= 2) {
            g_card_scan = CardPollPorts(0x81, 0x81);
            if (g_card_scan) {
                state = 1;
            }
        }
        OpenDrawTitle();
    }
    for (i = 0x80; i >= 0; i -= 4) {
        g_sprites[0].r = g_sprites[0].g = g_sprites[0].b = i;
        OpenDrawTitle();
    }
    g_card_scan = 0;
    goto out;
chosen:
    g_title_choice = g_title_cursor;
    g_card_scan = -1;
out:
    return g_card_scan;
}

void OpenDrawTitle(void)
{
    int i;

    g_active_buff = GsGetActiveBuff();
    GsSetWorkBase((PACKET *)g_packet[g_active_buff]);
    GsClearOt(0, 0, &g_ot[g_active_buff]);
    if (g_msg_len) {
        for (i = 0; i < g_msg_len; i++) {
            GsSortFastSprite(&g_sprites[i + 0x125], &g_ot[g_active_buff], g_sprite_flags[i + 0x125] & 0x7F);
        }
        GsSortBoxFill(&g_msg_box, &g_ot[g_active_buff], 0);
    }
    if (g_msg_box_shown) {
        GsSortBoxFill(&g_msg_box, &g_ot[g_active_buff], 0);
    }
    for (i = 0; i < 10; i++) {
        if (g_msg_len) {
            g_sprites[i].r = g_sprites[i].g = g_sprites[i].b = 0x20;
        } else {
            g_sprites[i].r = g_sprites[i].g = g_sprites[i].b = g_sprites[0].r;
        }
        if (g_sprite_flags[i] & 0x80) {
            GsSortFastSprite(&g_sprites[i], &g_ot[g_active_buff], g_sprite_flags[i] & 0x7F);
        }
    }
    VSync(2);
    g_pad_old = g_pad_now;
    g_pad_now = ~((g_pad_buf0[2] << 8) | g_pad_buf0[3]);
    g_pad_trig = g_pad_now ^ (g_pad_now & g_pad_old);
    GsSwapDispBuff();
    GsSortClear(0, 0, 0, &g_ot[g_active_buff]);
    GsDrawOt(&g_ot[g_active_buff]);
}

void OpenPlaySeq(int i)
{
    SsPlayBack(g_open_seq[i], 0, 1);
}

void OpenLoadTim(u_long *addr, int no_clut)
{
    RECT    rect;
    GsIMAGE image;

    GsGetTimInfo(addr + 1, &image);
    rect.x = image.px;
    rect.y = image.py;
    rect.w = image.pw;
    rect.h = image.ph;
    LoadImage(&rect, image.pixel);
    if (no_clut == 0 && (image.pmode >> 3) & 1) {
        rect.x = image.cx;
        rect.y = image.cy;
        rect.w = image.cw;
        rect.h = image.ch;
        LoadImage(&rect, image.clut);
    }
}

void OpenSpriteInit(u_short no, u_short w, u_short h, u_short tpage, u_short u, u_short v, u_short cx, u_short cy)
{
    g_sprites[no].attribute = 0x1000000;
    g_sprites[no].w = w;
    g_sprites[no].h = h;
    g_sprites[no].mx = w / 2;
    g_sprites[no].my = h / 2;
    g_sprites[no].tpage = tpage;
    g_sprites[no].u = u;
    g_sprites[no].v = v;
    g_sprites[no].cx = cx;
    g_sprites[no].cy = cy;
    g_sprites[no].r = 0x80;
    g_sprites[no].g = 0x80;
    g_sprites[no].b = 0x80;
    g_sprites[no].rotate = 0;
    g_sprites[no].scalex = 0x1000;
    g_sprites[no].scaley = 0x1000;
}

int OpenLoadMenu(int kind)
{
    int  i;
    int  port;
    int  bit;
    int  slot;
    int  yesno;
    RECT rect;
    int  want[2];

    SetPolyG4(&g_grad_poly);
    setXY4(&g_grad_poly, 0, 0, 0x13F, 0, 0, 0x77, 0x13F, 0x77);
    setRGB0(&g_grad_poly, 0, 0, 0xFF);
    setRGB1(&g_grad_poly, 0, 0, 0xFF);
    setRGB2(&g_grad_poly, 0, 0, 0);
    setRGB3(&g_grad_poly, 0, 0, 0);
    for (i = 0; i < 20; i++) {
        g_lines[i].attribute = 0;
        g_lines[i].x0 = g_lines[i].x1 = i * 16 + 8;
        g_lines[i].y0 = 0;
        g_lines[i].y1 = 0xF0;
        g_lines[i].r = 0x40;
        g_lines[i].g = 0x40;
        g_lines[i].b = 0x40;
    }
    for (i = 0; i < 15; i++) {
        g_lines[i + 20].attribute = 0;
        g_lines[i + 20].x0 = 0;
        g_lines[i + 20].x1 = 0x140;
        g_lines[i + 20].y0 = g_lines[i + 20].y1 = i * 16 + 8;
        g_lines[i + 20].r = 0x40;
        g_lines[i + 20].g = 0x40;
        g_lines[i + 20].b = 0x40;
    }
    g_pad_now = -1;
    g_pad_trig = 0;
    for (i = 0; i < 0x103; i++) {
        g_sprite_flags[i + 10] = 0;
    }
    OpenLoadMenuSprites(kind);
    OpenFontLoadText(12, 2, 0x40, 0x54, g_txt_load_header);
    g_menu_state = 0;
    port = 0;
    while (1) {
        switch (g_menu_state) {
        case 0:
            if (g_pad_trig & 0x5000) {
                OpenPlaySeq(0);
                port ^= 1;
            } else if (g_pad_trig & 0x40) {
                OpenPlaySeq(2);
                return 1;
            } else if (g_pad_trig & 0x80) {
                OpenPlaySeq(2);
                return 1;
            } else if (g_pad_trig & 0x820) {
                OpenPlaySeq(1);
                VSync(30);
                g_card_scan = CardLoad(port);
                if (g_card_scan) {
                    OpenCardPrompt(g_card_scan, port, kind);
                } else {
                    VSync(0);
                    g_card_scan = CardScanSaves(port, kind, g_save_list);
                    if (g_card_scan == -1) {
                        OpenMessageOpen(12, 1, 0x40, 0x5C, g_txt_card_error);
                        VSync(0);
                        for (;;) {
                            if (CardPollPorts(port == 0 ? 0x81 : 0, port ? 0x81 : 0)) {
                                OpenLoadMenuReset(kind);
                                break;
                            }
                            OpenDrawMenu();
                            if (g_pad_trig) {
                                break;
                            }
                        }
                        g_msg_len = 0;
                    } else {
                        bit = 0x100;
                        for (i = 0; i < 7; i++) {
                            if (g_card_scan & bit) {
                                OpenMessageOpen(12, 2, 0x40, 0x54, g_txt_save_broken);
                                OpenFontRenderGlyph(i + 0xC1);
                                rect.x = 0x354;
                                rect.y = 0x100;
                                rect.w = 4;
                                rect.h = 16;
                                LoadImage(&rect, (u_long *)g_font_glyph);
                                DrawSync(0);
                                VSync(0);
                                for (;;) {
                                    if (CardPollPorts(port == 0 ? 0x81 : 0, port ? 0x81 : 0)) {
                                        OpenLoadMenuReset(kind);
                                        break;
                                    }
                                    OpenDrawMenu();
                                    if (g_pad_trig) {
                                        break;
                                    }
                                }
                            }
                            bit <<= 1;
                        }
                        if ((want[port] = (g_card_scan & 0x7F) ^ ((g_card_scan & 0x7F00) >> 8))) {
                            want[port ^ 1] = 0;
                            g_msg_len = 0;
                            g_menu_state = 1;
                            g_text_len = 0;
                            g_pad_now = -1;
                            g_pad_trig = 0;
                            bit = 1;
                            for (i = 0; i < 7; i++) {
                                if ((g_card_scan & bit) != ((bit << 8) & g_card_scan) >> 8) {
                                    break;
                                }
                                bit <<= 1;
                            }
                            OpenSaveListInit(port, g_card_scan, slot = i);
                            break;
                        } else {
                            OpenMessageOpen(12, 1, 0x40, 0x5C, g_txt_no_saves);
                            OpenPlaySeq(3);
                            VSync(0);
                            for (;;) {
                                if (CardPollPorts(want[0], want[1])) {
                                    OpenLoadMenuReset(kind);
                                    break;
                                }
                                OpenDrawMenu();
                                if (g_pad_trig) {
                                    break;
                                }
                            }
                            g_msg_len = 0;
                        }
                    }
                }
            }
            g_sprites[15].y = port * 16 + 0x92;
            break;
        case 1:
            if (g_pad_trig & 0x1000) {
                OpenPlaySeq(0);
                slot--;
            } else if (g_pad_trig & 0x4000) {
                OpenPlaySeq(0);
                slot++;
            } else if (g_pad_trig & 0x40) {
                OpenPlaySeq(2);
                OpenLoadMenuReset(kind);
                g_sprites[15].y = port * 16 + 0x92;
                break;
            } else if (g_pad_trig & 0x80) {
                OpenPlaySeq(2);
                return 1;
            } else if (g_pad_trig & 0x820) {
                OpenPlaySeq(1);
                VSync(30);
                g_card_status = CardCheckSave(port, kind, slot);
                if (g_card_status == 0) {
                    OpenMessageOpen(12, 1, 0x40, 0x5C, g_txt_no_saves);
                    VSync(0);
                    for (;;) {
                        if (CardPollPorts(want[0], want[1])) {
                            OpenLoadMenuReset(kind);
                            break;
                        }
                        OpenDrawMenu();
                        if (g_pad_trig) {
                            break;
                        }
                    }
                    g_msg_len = 0;
                    break;
                } else if (g_card_status == -2 || g_card_status == -1) {
                    OpenMessageOpen(12, 2, 0x40, 0x54, g_txt_save_broken);
                    OpenFontRenderGlyph(slot + 0xC1);
                    rect.x = 0x354;
                    rect.y = 0x100;
                    rect.w = 4;
                    rect.h = 16;
                    LoadImage(&rect, (u_long *)g_font_glyph);
                    DrawSync(0);
                    VSync(0);
                    for (;;) {
                        if (CardPollPorts(want[0], want[1])) {
                            break;
                        }
                        OpenDrawMenu();
                        if (g_pad_trig) {
                            break;
                        }
                    }
                    g_msg_len = 0;
                    OpenLoadMenuReset(kind);
                    break;
                }
                if (kind == 1) {
                    OpenMessageOpen(13, 3, 0x38, 0x54, g_txt_suspend_erase);
                    VSync(0);
                    for (;;) {
                        if (CardPollPorts(want[0], want[1])) {
                            OpenLoadMenuReset(kind);
                            for (i = 0xFC; i < 0x103; i++) {
                                g_sprite_flags[i + 10] = 0;
                            }
                            g_msg_len = 0;
                            goto next;
                        }
                        OpenDrawMenu();
                        if (g_pad_trig) {
                            break;
                        }
                    }
                    g_msg_len = 0;
                }
                OpenConfirmOpen();
                yesno = 0;
            next:
                break;
            }
            slot = slot < 0 ? 6 : slot;
            slot = slot < 7 ? slot : 0;
            g_sprites[0x17].y = SAVE_ROW_Y(slot);
            break;
        case 2:
            if (g_pad_trig & 0x5000) {
                OpenPlaySeq(0);
                yesno ^= 1;
            } else if (g_pad_trig & 0x40) {
                OpenPlaySeq(2);
                g_menu_state = 1;
                for (i = 0xFC; i < 0x103; i++) {
                    g_sprite_flags[i + 10] = 0;
                }
                g_pad_now = -1;
                g_pad_trig = 0;
                break;
            } else if (g_pad_trig & 0x80) {
                OpenPlaySeq(2);
                return 1;
            } else if (g_pad_trig & 0x820) {
                if (yesno == 1) {
                    OpenPlaySeq(2);
                    g_menu_state = 1;
                    for (i = 0xFC; i < 0x103; i++) {
                        g_sprite_flags[i + 10] = 0;
                    }
                    g_pad_now = -1;
                    g_pad_trig = 0;
                    break;
                }
                OpenMessageOpen(12, 3, 0x40, 0x58, g_txt_loading);
                OpenDrawMenu();
                OpenDrawMenu();
                g_load_status = CardLoadSave(port, slot, kind);
                if (g_load_status) {
                    goto failed;
                }
                if (kind == 1) {
                    VSync(0);
                    g_load_status = CardDeleteFile(port, slot, kind);
                    if (g_load_status == 0) {
                        goto loaded;
                    }
                }
            failed:
                g_msg_len = 0;
                VSync(0);
                g_card_status = CardLoad(port);
                if (g_card_status) {
                    OpenCardPrompt(g_card_status, port, kind);
                    OpenLoadMenuReset(kind);
                    break;
                }
                if (g_load_status == -2 || g_load_status == -1) {
                    OpenMessageOpen(12, 2, 0x40, 0x54, g_txt_save_broken);
                    OpenFontRenderGlyph(slot + 0xC1);
                    rect.x = 0x354;
                    rect.y = 0x100;
                    rect.w = 4;
                    rect.h = 16;
                    LoadImage(&rect, (u_long *)g_font_glyph);
                    DrawSync(0);
                    for (;;) {
                        if (CardPollPorts(want[0], want[1])) {
                            break;
                        }
                        OpenDrawMenu();
                        if (g_pad_trig) {
                            break;
                        }
                    }
                    g_msg_len = 0;
                    g_menu_state = 1;
                    for (i = 0xFC; i < 0x103; i++) {
                        g_sprite_flags[i + 10] = 0;
                    }
                    g_pad_now = -1;
                    g_pad_trig = 0;
                    break;
                }
            loaded:
                g_save_slot = slot;
                g_save_chan = port;
                g_msg_len = 0;
                return 0;
            }
            g_sprites[0x106].y = yesno * 16 + 0xBA;
            break;
        }
        if (g_menu_state != 0 && CardPollPorts(want[0], want[1])) {
            OpenLoadMenuReset(kind);
            g_sprites[15].y = port * 16 + 0x92;
        }
        OpenDrawMenu();
    }
}

void OpenConfirmOpen(void)
{
    g_menu_state = 2;
    OpenConfirmSprites();
    g_pad_now = -1;
    g_pad_trig = 0;
}

void OpenLoadMenuReset(int arg0)
{
    int i;

    g_menu_state = 0;
    for (i = 0; i < 0x103; i++) {
        g_sprite_flags[i + 10] = 0;
    }
    OpenLoadMenuSprites(arg0);
    OpenFontLoadText(12, 2, 0x40, 0x54, g_txt_load_header);
    g_pad_now = -1;
    g_pad_trig = 0;
}

void OpenDrawMenu(void)
{
    int i;

    g_active_buff = GsGetActiveBuff();
    GsSetWorkBase((PACKET *)g_packet[g_active_buff]);
    GsClearOt(0, 0, &g_ot[g_active_buff]);
    if (g_msg_len) {
        for (i = 0; i < g_msg_len; i++) {
            GsSortFastSprite(&g_sprites[i + 0x125], &g_ot[g_active_buff], g_sprite_flags[i + 0x125] & 0x7F);
        }
        GsSortBoxFill(&g_msg_box, &g_ot[g_active_buff], 0);
    }
    for (i = 0; i < g_text_len; i++) {
        if (g_msg_len) {
            g_sprites[i + 0x10D].r = g_sprites[i + 0x10D].g = g_sprites[i + 0x10D].b = 0x20;
        } else {
            g_sprites[i + 0x10D].r = g_sprites[i + 0x10D].g = g_sprites[i + 0x10D].b = 0x80;
        }
        GsSortFastSprite(&g_sprites[i + 0x10D], &g_ot[g_active_buff], g_sprite_flags[i + 0x10D] & 0x7F);
    }
    for (i = 0; i < 0x103; i++) {
        if (g_msg_len) {
            g_sprites[i + 10].r = g_sprites[i + 10].g = g_sprites[i + 10].b = 0x20;
        } else {
            g_sprites[i + 10].r = g_sprites[i + 10].g = g_sprites[i + 10].b = 0x80;
        }
        if (g_sprite_flags[i + 10] & 0x80) {
            GsSortFastSprite(&g_sprites[i + 10], &g_ot[g_active_buff], g_sprite_flags[i + 10] & 0x7F);
        }
    }
    for (i = 0; i < 35; i++) {
        GsSortLine(&g_lines[i], &g_ot[g_active_buff], 15);
    }
    GsSortPoly(&g_grad_poly, &g_ot[g_active_buff], 15);
    VSync(2);
    g_pad_old = g_pad_now;
    g_pad_now = ~((g_pad_buf0[2] << 8) | g_pad_buf0[3]);
    g_pad_trig = g_pad_now ^ (g_pad_now & g_pad_old);
    GsSwapDispBuff();
    GsSortClear(0, 0, 0, &g_ot[g_active_buff]);
    GsDrawOt(&g_ot[g_active_buff]);
}

void OpenLoadMenuSprites(int alt)
{
    OpenSpriteInit(10, 0x30, 0x10, 0x1B, 0x90, 0, 0x100, 0x1E1);
    if (alt) {
        g_sprites[10].u = 0xC0;
    }
    g_sprites[10].attribute = 0;
    g_sprites[10].x = 0x10;
    g_sprites[10].y = 0x18;
    g_sprite_flags[10] = 0x80;
    OpenSpriteInit(11, 0x20, 0x20, 0x19, 0, 0, 0, 0x1E1);
    g_sprites[11].x = 0x18;
    g_sprites[11].y = 0x10;
    g_sprite_flags[11] = 0x80;
    OpenSpriteInit(12, 0xE0, 0x48, 0x19, 0, 0x20, 0, 0x1E1);
    g_sprites[12].x = 0x30;
    g_sprites[12].y = 0x40;
    g_sprite_flags[12] = 0x80;
    OpenSpriteInit(13, 0x30, 0x10, 0x1B, 0, 0x10, 0x100, 0x1E1);
    g_sprites[13].attribute = 0;
    g_sprites[13].x = 0xE0;
    g_sprites[13].y = 0x90;
    g_sprite_flags[13] = 0x80;
    OpenSpriteInit(14, 0x30, 0x10, 0x1B, 0x30, 0x10, 0x100, 0x1E1);
    g_sprites[14].attribute = 0;
    g_sprites[14].x = 0xE0;
    g_sprites[14].y = 0xA0;
    g_sprite_flags[14] = 0x80;
    OpenSpriteInit(15, 0x30, 0xC, 0x18, 0, 0xC8, 0x100, 0x1E0);
    g_sprites[15].attribute = 0x40000000;
    g_sprites[15].x = 0xE0;
    g_sprites[15].y = 0x92;
    g_sprite_flags[15] = 0x80;
    OpenSpriteInit(16, 0x30, 0x10, 0x19, 0x20, 0xC, 0, 0x1E1);
    g_sprites[16].x = 0xE0;
    g_sprites[16].y = 0x90;
    g_sprite_flags[16] = 0x80;
    OpenSpriteInit(17, 0x30, 0x10, 0x19, 0x20, 0xC, 0, 0x1E1);
    g_sprites[17].x = 0xE0;
    g_sprites[17].y = 0xA0;
    g_sprite_flags[17] = 0x80;
}

void OpenSaveListInit(int kind, int mask, int cursor)
{
    int i;
    int j;
    int bit;

    OpenSpriteInit(12, 0x30, 0x10, 0x1B, kind * 48, 0x10, 0x100, 0x1E1);
    g_sprites[12].attribute = 0;
    g_sprites[12].x = 0x48;
    g_sprites[12].y = 0x10;
    g_sprite_flags[12] = 0x82;
    OpenSpriteInit(13, 0x30, 0x10, 0x19, 0x20, 0xC, 0, 0x1E1);
    g_sprites[13].x = 0x48;
    g_sprites[13].y = 0x10;
    g_sprite_flags[13] = 0x82;
    OpenSpriteInit(14, 0xB0, 0xC, 0x19, 0, 0x68, 0, 0x1E1);
    g_sprites[14].x = 0x48;
    g_sprites[14].y = 0x18;
    g_sprite_flags[14] = 0x82;
    for (i = 0; i < 7; i++) {
        OpenSpriteInit(i + 15, 0xB0, 0x18, 0x19, 0, 0x74, 0, 0x1E1);
        g_sprites[i + 15].x = 0x48;
        g_sprites[i + 15].y = SAVE_ROW_Y(i);
        g_sprite_flags[i + 15] = 0x82;
    }
    OpenSpriteInit(0x16, 0xB0, 0xC, 0x19, 0, 0x8C, 0, 0x1E1);
    g_sprites[0x16].x = 0x48;
    g_sprites[0x16].y = 0xCC;
    g_sprite_flags[0x16] = 0x82;
    OpenSpriteInit(0x17, 0xA0, 0x18, 0x18, 0, 0xD8, 0x100, 0x1E0);
    g_sprites[0x17].attribute = 0x40000000;
    g_sprites[0x17].x = 0x50;
    g_sprites[0x17].y = SAVE_ROW_Y(cursor);
    g_sprite_flags[0x17] = 0x81;
    for (i = 0; i < 7; i++) {
        for (j = 0; j < 34; j++) {
            OpenSpriteInit(j + 24 + i * 34, 8, 12, 0x18, 0, 0, 0x100, 0x1E0);
            g_sprites[FONT_CELL(i, j + 24)].attribute = 0;
            g_sprites[FONT_CELL(i, j + 24)].x = j % 17 * 8 + 0x50;
            g_sprites[FONT_CELL(i, j + 24)].y = j / 17 * 12 + SAVE_ROW_Y(i);
        }
        OpenSpriteSetUV(FONT_CELL(i, 24), 0xD8, 0x48);
        OpenSpriteSetUV(FONT_CELL(i, 33), 0xE0, 0x48);
        OpenSpriteSetUV(FONT_CELL(i, 41), 8, 0x78);
        OpenSpriteSetUV(FONT_CELL(i, 42), 0x10, 0x78);
        OpenSpriteSetUV(FONT_CELL(i, 48), 0x10, 0x6C);
        OpenSpriteSetUV(FONT_CELL(i, 49), 0x18, 0x6C);
        OpenSpriteSetUV(FONT_CELL(i, 50), 0x20, 0x6C);
        OpenSpriteSetUV(FONT_CELL(i, 51), 0x28, 0x6C);
        OpenSpriteSetUV(FONT_CELL(i, 55), 0x88, 0x48);
    }
    bit = 1;
    for (i = 0; i < 7; i++) {
        if ((mask & bit) && !((bit << 8) & mask)) {
            for (j = 0; j < 34; j++) {
                g_sprite_flags[i * 34 + j + 24] = 0x80;
            }
            OpenFontPutText(i, g_save_list[i].name);
            OpenFontPutNumber(i, g_save_list[i].level, 20);
            OpenFontPutNumber(i, g_save_list[i].hours, 29);
            OpenFontPutNumber(i, g_save_list[i].minutes, 32);
        }
        bit <<= 1;
    }
}

void OpenConfirmSprites(void)
{
    OpenSpriteInit(0x106, 0x30, 0xC, 0x18, 0, 0xC8, 0x100, 0x1E0);
    g_sprites[0x106].attribute = 0x40000000;
    g_sprites[0x106].x = 0x100;
    g_sprites[0x106].y = 0xBA;
    g_sprite_flags[0x106] = 0x81;
    OpenSpriteInit(0x107, 0x30, 0x10, 0x1B, 0, 0, 0x100, 0x1E1);
    g_sprites[0x107].attribute = 0;
    g_sprites[0x107].x = 0x100;
    g_sprites[0x107].y = 0xA8;
    g_sprite_flags[0x107] = 0x80;
    OpenSpriteInit(0x108, 0x30, 0x10, 0x1B, 0x30, 0, 0x100, 0x1E1);
    g_sprites[0x108].attribute = 0;
    g_sprites[0x108].x = 0x100;
    g_sprites[0x108].y = 0xB8;
    g_sprite_flags[0x108] = 0x80;
    OpenSpriteInit(0x109, 0x30, 0x10, 0x1B, 0x60, 0, 0x100, 0x1E1);
    g_sprites[0x109].attribute = 0;
    g_sprites[0x109].x = 0x100;
    g_sprites[0x109].y = 0xC8;
    g_sprite_flags[0x109] = 0x80;
    OpenSpriteInit(0x10A, 0x30, 0x10, 0x19, 0x20, 0xC, 0, 0x1E1);
    g_sprites[0x10A].x = 0x100;
    g_sprites[0x10A].y = 0xA8;
    g_sprite_flags[0x10A] = 0x82;
    OpenSpriteInit(0x10B, 0x30, 0x10, 0x19, 0x20, 0xC, 0, 0x1E1);
    g_sprites[0x10B].x = 0x100;
    g_sprites[0x10B].y = 0xB8;
    g_sprite_flags[0x10B] = 0x82;
    OpenSpriteInit(0x10C, 0x30, 0x10, 0x19, 0x20, 0xC, 0, 0x1E1);
    g_sprites[0x10C].x = 0x100;
    g_sprites[0x10C].y = 0xC8;
    g_sprite_flags[0x10C] = 0x82;
}

void OpenFontPutText(int row, u_char *text)
{
    int i;

    for (i = 0; i < 8; i++) {
        if (*text == 0xFF) {
            break;
        }
        OpenSpriteSetUV(FONT_CELL(row, i + 25), *text % 31 * 8, *text / 31 * 12);
        text++;
    }
}

void OpenFontPutNumber(int row, int value, int col)
{
    if (value / 10 != 0 || col == 32) {
        OpenSpriteSetUV(FONT_CELL(row, col + 24), value / 10 * 8 + 0x30, 0x48);
    }
    OpenSpriteSetUV(FONT_CELL(row, col + 25), value % 10 * 8 + 0x30, 0x48);
}

void OpenSpriteSetUV(no, u, v)
u_short no, u, v;
{
    g_sprites[no].u = u;
    g_sprites[no].v = v;
}

void OpenFontLoadText(short w, short h, short x, short y, u_short *text)
{
    RECT rect;
    int  i;
    int  j;

    rect.w = 4;
    rect.h = 16;
    g_text_len = w * h;
    for (i = 0; i < h; i++) {
        for (j = 0; j < w; j++) {
            OpenFontRenderGlyph(*text);
            rect.x = j * 4 + 0x300;
            rect.y = i * 16 + 0x100;
            LoadImage(&rect, (u_long *)g_font_glyph);
            DrawSync(0);
            text++;
        }
    }
    for (i = 0; i < h; i++) {
        for (j = 0; j < w; j++) {
            OpenSpriteInit(w * i + 0x10D + j, 16, 16, 0x1C, j * 16, i * 16, 0x100, 0x1E2);
            g_sprites[w * i + 0x10D + j].attribute = 0;
            g_sprites[w * i + 0x10D + j].x = x + j * 16;
            g_sprites[w * i + 0x10D + j].y = y + i * 16;
            g_sprite_flags[w * i + j + 0x10D] = 0;
        }
    }
}

void OpenCardPrompt(int kind, int port, int arg2)
{
    OpenPlaySeq(3);
    switch (kind) {
    case 1:
        OpenMessageOpen(12, 1, 0x40, 0x5C, g_txt_card_error);
        break;
    case 2:
        OpenMessageOpen(13, 1, 0x38, 0x58, g_txt_no_card);
        break;
    case 4:
        OpenMessageOpen(13, 2, 0x38, 0x54, g_txt_unformatted);
        break;
    }
    OpenDrawMenu();
    CardPollPorts(0, 0);
    for (;;) {
        if (CardPollPorts(port == 0 ? 0x81 : 0, port ? 0x81 : 0)) {
            OpenLoadMenuReset(arg2);
            break;
        }
        OpenDrawMenu();
        if (g_pad_trig) {
            break;
        }
    }
    g_msg_len = 0;
}

void OpenMessageOpen(short w, short h, short x, short y, u_short *text)
{
    RECT rect;
    int  i;
    int  j;

    rect.w = 4;
    rect.h = 16;
    OpenPlaySeq(3);
    g_msg_len = w * h;
    for (i = 0; i < h; i++) {
        for (j = 0; j < w; j++) {
            OpenFontRenderGlyph(*text);
            rect.x = j * 4 + 0x340;
            rect.y = i * 16 + 0x100;
            LoadImage(&rect, (u_long *)g_font_glyph);
            DrawSync(0);
            text++;
        }
    }
    for (i = 0; i < h; i++) {
        for (j = 0; j < w; j++) {
            OpenSpriteInit(w * i + 0x125 + j, 16, 16, 0x1D, j * 16, i * 16, 0x100, 0x1E2);
            g_sprites[w * i + 0x125 + j].attribute = 0;
            g_sprites[w * i + 0x125 + j].x = x + j * 16;
            g_sprites[w * i + 0x125 + j].y = y + i * 16;
            g_sprite_flags[w * i + j + 0x125] = 0;
        }
    }
    g_msg_box.attribute = 0x40000000;
    g_msg_box.x = x - 4;
    g_msg_box.y = y - 4;
    g_msg_box.w = w * 16 + 8;
    g_msg_box.h = h * 16 + 8;
    g_msg_box.r = 0;
    g_msg_box.g = 0x80;
    g_msg_box.b = 0;
}

void OpenFontRenderGlyph(code)
u_short code;
{
    u_char *p;
    u_char  bits;
    u_long  pix[2];
    u_long  carry;
    int     j;
    int     row;
    int     k;

    p = (u_char *)0x801E001F + code * 32;
    for (row = 15; row > -1; row--) {
        pix[0] = pix[1] = 0;
        for (j = 1; j > -1; j--) {
            bits = *p--;
            for (k = 7; k > -1; k--) {
                pix[j] |= (bits & 1) << (k * 4);
                bits >>= 1;
            }
        }
        g_font_glyph[row][0] = pix[0];
        g_font_glyph[row][1] = pix[1];
        if (row < 15) {
            carry = (g_font_glyph[row][0] & 0xF8000000) >> 27;
            g_font_glyph[row + 1][0] |= g_font_glyph[row][0] << 5;
            g_font_glyph[row + 1][1] |= (g_font_glyph[row][1] << 5) | carry;
        }
    }
}
