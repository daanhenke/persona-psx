#include <decomp/common.h>
/* Persona 1 (JP) - the name-entry screen.  NAME @ 0x800643A4.
 */
#include <decomp/types.h>
#include <libetc.h>
#include <libsnd.h>
#include <memory.h>
#include <persona/name/entry.h>

void NameEntryInit(void);
void NameDrawFrame(void);
int  NameEntryInput(void);

extern u_char g_member_names[];
extern u_char g_text_hero_name[8];

void NameCaretPlace(void);

/* Where the save keeps the names: the two short ones as text, run together,
   and the eight-cell one on its own. */
#define SAVE_NAME0  ((u_char *)0x801F298C)
#define SAVE_NAME1  ((u_char *)0x801F2996)
#define SAVE_HERO   ((u_char *)0x801F29A0)
#define SAVE_HERO2  ((u_char *)0x801F1C0B)

void ovl_name_entry(void)
{
    int i;
    int k;
    int j;
    int len;

    NameEntryInit();
    for (i = 0; i < 0x81; i++) {
        for (k = 0; k < NAME_SPRITES; k++) {
            g_sprites[k].r = g_sprites[k].g = g_sprites[k].b = i;
        }
        g_bg_list.r = g_bg_list.g = g_bg_list.b = i;
        g_bg_keys.r = g_bg_keys.g = g_bg_keys.b = i;
        g_bg_frame.r = g_bg_frame.g = g_bg_frame.b = i;
        g_bg_fields.r = g_bg_fields.g = g_bg_fields.b = i;
        NameDrawFrame();
    }
    while (NameEntryInput()) {
        NameDrawFrame();
    }
    for (i = 0; i < 0x81; i++) {
        for (k = 0; k < NAME_SPRITES; k++) {
            if (g_sprites[k].r) {
                g_sprites[k].r = g_sprites[k].g = g_sprites[k].b = g_sprites[k].b - 1;
            }
        }
        if (g_bg_list.r) {
            g_bg_list.r = g_bg_list.g = g_bg_list.b = g_bg_list.b - 1;
        }
        if (g_bg_keys.r) {
            g_bg_keys.r = g_bg_keys.g = g_bg_keys.b = g_bg_keys.b - 1;
        }
        if (g_bg_frame.r) {
            g_bg_frame.r = g_bg_frame.g = g_bg_frame.b = g_bg_frame.b - 1;
        }
        if (g_bg_fields.r) {
            g_bg_fields.r = g_bg_fields.g = g_bg_fields.b = g_bg_fields.b - 1;
        }
        NameDrawFrame();
    }
    SetDispMask(0);

#ifdef VER_US
    /* One name, in the save's own letter codes: 0x30 above the font's. */
    for (i = 0; i < NAME_CELLS; i++) {
        if (g_name_text[2][i] != 0) {
            SAVE_HERO[i] = g_name_text[2][i] + 0x30;
        }
    }
#else
    for (i = NAME_SHORT - 1; i >= 0; i--) {
        if (g_name_text[0][i] != 0) {
            len = i + 1;
            break;
        }
    }
    j = 0;
    for (i = 0; i < len; i++) {
        if (g_name_text[0][i] >= 0x80) {
            SAVE_NAME0[j++] = (g_name_text[0][i] >> 8) | 0x80;
        }
        SAVE_NAME0[j++] = g_name_text[0][i];
    }
    if (len != NAME_SHORT) {
        SAVE_NAME0[j] = 0xFF;
        SAVE_NAME0[j + 1] = 1;
    }

    for (i = NAME_SHORT - 1; i >= 0; i--) {
        if (g_name_text[1][i] != 0) {
            len = i + 1;
            break;
        }
    }
    j = 0;
    for (i = 0; i < len; i++) {
        if (g_name_text[1][i] >= 0x80) {
            SAVE_NAME1[j++] = (g_name_text[1][i] >> 8) | 0x80;
        }
        SAVE_NAME1[j++] = g_name_text[1][i];
    }
    if (len != NAME_SHORT) {
        SAVE_NAME1[j] = 0xFF;
        SAVE_NAME1[j + 1] = 1;
    }

    for (i = 0; i < NAME_CELLS; i++) {
        SAVE_HERO[i] = g_name_text[2][i];
        if (SAVE_HERO[i] == 0xCC || SAVE_HERO[i] == 0xD4 || SAVE_HERO[i] == 0xE1) {
            SAVE_HERO[i] = 0xCC;
        }
    }
#endif
    if (SAVE_HERO[NAME_CELLS - 1] == 0) {
        for (i = NAME_CELLS - 2; i >= 0; i--) {
            if (SAVE_HERO[i] != 0) {
                SAVE_HERO[i + 1] = 0xFF;
                break;
            }
        }
    }
#ifndef VER_US
    bcopy(SAVE_NAME0, g_member_names, 0x14);
#endif
    bcopy(SAVE_HERO, SAVE_HERO2, 8);
    bcopy(SAVE_HERO, g_text_hero_name, 8);
    VSync(0x78);
}

#define CUR        g_name_cursor[g_name_field]
#define CARET_CHAR g_name_text[g_name_field][g_name_cursor[g_name_field]]

int  NameAddDakuten(u_short code, signed char col);
int  NameEntryComplete(void);
int  NameConfirm(void);
void NameListScroll(u_char back);
void NameListFill(u_char group, u_char row);
void NameListPreview(void);
void NameListClose(void);
void NameSetPage();

#ifdef VER_US
/* One frame of input. Answers 0 once the name is accepted.

   US keeps only the keyboard: one page of letters, the buttons in the column
   after them, and no kanji list. */
int NameEntryInput(void)
{
    int i;

    if (g_name_repeat != 0) {
        return 1;
    }
    if (g_pad_held & PAD_UP) {
        NamePlaySe(0);
        if (--g_name_row < 0) {
            g_name_row = 5;
        } else if (g_name_row == 4 && g_name_col == KEY_COLS) {
            g_name_row = 3;
        }
    } else if (g_pad_held & PAD_DOWN) {
        NamePlaySe(0);
        if (++g_name_row >= 6) {
            g_name_row = 0;
        } else if (g_name_row == 4 && g_name_col == KEY_COLS) {
            g_name_row = 5;
        }
    } else if (g_pad_held & PAD_LEFT) {
        NamePlaySe(0);
        if (--g_name_col < 0) {
            g_name_col = KEY_COLS;
            if (g_name_row == 4) {
                g_name_row = 3;
            }
        }
    } else if (g_pad_held & PAD_RIGHT) {
        NamePlaySe(0);
        if (++g_name_col >= KEY_COLS + 1) {
            g_name_col = 0;
        }
        if (g_name_col == KEY_COLS && g_name_row == 4) {
            g_name_row = 3;
        }
    } else if (g_pad_held & PAD_L1) {
        if (CUR > 0) {
            NamePlaySe(0);
            CUR--;
        } else if (g_pad_trig & PAD_L1) {
            NamePlaySe(0);
        }
    } else if (g_pad_held & PAD_R1) {
        if (CUR < g_name_len[g_name_field] - 1) {
            NamePlaySe(0);
            CUR++;
        } else if (g_pad_trig & PAD_R1) {
            NamePlaySe(0);
        }
    } else if (g_pad_held & PAD_OK) {
        if (g_name_col != KEY_COLS) {
            CARET_CHAR = g_keyboard[g_name_page][g_name_row][g_name_col];
            NameDrawCursor();
            if (CUR != g_name_len[g_name_field] - 1) {
                NamePlaySe(1);
                CUR++;
            } else if (g_pad_trig & PAD_OK) {
                NamePlaySe(1);
            }
        } else if (g_name_row < 2) {
            if (g_name_row == 0) {
                if (CUR > 0) {
                    NamePlaySe(1);
                    CUR--;
                } else if (g_pad_trig & PAD_OK) {
                    NamePlaySe(1);
                }
            } else if (CUR < g_name_len[g_name_field] - 1) {
                NamePlaySe(1);
                CUR++;
            } else if (g_pad_trig & PAD_OK) {
                NamePlaySe(1);
            }
        } else if (g_name_row == 2) {
            if (CUR != 0) {
                NamePlaySe(3);
                if (CARET_CHAR == 0) {
                    CUR--;
                }
            } else if (g_name_text[g_name_field][0] != 0 || (g_pad_trig & PAD_OK)) {
                NamePlaySe(3);
            }
            CARET_CHAR = 0;
            NameDrawCursor();
        } else if (g_name_row == 3) {
            if (g_pad_trig & PAD_OK) {
                NamePlaySe(3);
                for (i = NAME_CELLS - 1; i >= 0; i--) {
                    g_name_text[g_name_field][i] = 0;
                }
                for (i = 0; i < g_name_len[g_name_field]; i++) {
                    ExpandGlyph(g_name_text[g_name_field][i], &g_glyph_cell[i * 2],
                                g_name_len[g_name_field] * 2);
                }
                UploadImage(g_name_x[g_name_field] + 0x200, g_name_y[g_name_field],
                            g_name_len[g_name_field] * 4, 0x10, g_glyph_cell);
                CUR = 0;
            }
        } else if (g_pad_trig & PAD_OK) {
            if (NameEntryComplete()) {
                NamePlaySe(1);
                if (NameConfirm() == 0) {
                    NamePlaySe(1);
                    SsSeqSetDecrescendo(g_seq_handles[0], 0x7F, 0x78);
                    return 0;
                }
            }
            NamePlaySe(2);
        }
    } else if (g_pad_held & PAD_BACK) {
        if (CUR != 0) {
            NamePlaySe(2);
            if (CARET_CHAR == 0) {
                CUR--;
            }
        } else if (g_name_text[g_name_field][0] != 0 || (g_pad_trig & PAD_BACK)) {
            NamePlaySe(2);
        }
        CARET_CHAR = 0;
        NameDrawCursor();
    }

    if (g_name_col == KEY_COLS) {
        g_caret_x = 0x100;
    } else {
        g_caret_x = g_name_col * 16 + 0x18;
    }
    g_caret_y = g_name_row * 16 + 0x78;
    NameCaretPlace();
    return 1;
}
#else
/* One frame of input. Answers 0 once the name is accepted. */
int NameEntryInput(void)
{
    int i;
    int page;
    int slot;
    int top;

    switch (g_name_mode) {
    case 0:
        if (g_name_repeat != 0) {
            return 1;
        }
        if (g_pad_held & PAD_UP) {
            NamePlaySe(0);
            if (--g_name_row < 0) {
                g_name_row = 5;
            } else if (g_name_row == 4 && (g_name_col == 0 || g_name_col == 11)) {
                g_name_row = 3;
            }
        } else if (g_pad_held & PAD_DOWN) {
            NamePlaySe(0);
            if (++g_name_row >= 6) {
                g_name_row = 0;
            } else if (g_name_row == 4 && (g_name_col == 0 || g_name_col == 11)) {
                g_name_row = 5;
            }
        } else if (g_pad_held & PAD_LEFT) {
            NamePlaySe(0);
            if (--g_name_col < 0) {
                g_name_col = 11;
            } else if (g_name_col == 0 && g_name_row == 4) {
                g_name_row = 5;
            }
        } else if (g_pad_held & PAD_RIGHT) {
            NamePlaySe(0);
            if (++g_name_col >= 12) {
                g_name_col = 0;
            } else if (g_name_col == 11 && g_name_row == 4) {
                g_name_row = 5;
            }
        } else if (g_pad_held & PAD_L1) {
            if (CUR > 0) {
                NamePlaySe(0);
                CUR--;
            } else if (g_pad_trig & PAD_L1) {
                NamePlaySe(0);
            }
        } else if (g_pad_held & PAD_R1) {
            if (CUR < g_name_len[g_name_field] - 1) {
                NamePlaySe(0);
                CUR++;
            } else if (g_pad_trig & PAD_R1) {
                NamePlaySe(0);
            }
        } else if (g_pad_held & PAD_CIRCLE) {
            switch (g_name_col) {
            default:
                if (g_name_page != 3) {
                    if (g_name_page < 2 && g_name_row == 4 && (g_name_col == 9 || g_name_col == 10)) {
                        NamePlaySe(1);
                        if (CARET_CHAR != 0) {
                            if (NameAddDakuten(CARET_CHAR, g_name_col) != 0 && CUR > 0) {
                                CUR--;
                                if (CARET_CHAR != 0) {
                                    NameAddDakuten(CARET_CHAR, g_name_col);
                                }
                                CUR++;
                            }
                        } else if (CUR > 0) {
                            CUR--;
                            if (CARET_CHAR != 0) {
                                NameAddDakuten(CARET_CHAR, g_name_col);
                            }
                            CUR++;
                        }
                    } else {
                        CARET_CHAR = g_keyboard[g_name_page][g_name_row][g_name_col - 1];
                        NameDrawCursor();
                        if (CUR != g_name_len[g_name_field] - 1) {
                            NamePlaySe(1);
                            CUR++;
                        } else if (g_pad_trig & PAD_CIRCLE) {
                            NamePlaySe(1);
                        }
                    }
                } else {
                    if ((g_list_code = g_keyboard[3][g_name_row][g_name_col - 1]) == 0) {
                        CARET_CHAR = 0;
                        NameDrawCursor();
                        if (CUR != g_name_len[g_name_field] - 1) {
                            NamePlaySe(1);
                            CUR++;
                        } else if (g_pad_trig & PAD_CIRCLE) {
                            NamePlaySe(1);
                        }
                    } else {
                        NamePlaySe(1);
                        g_name_list_pos = 0;
                        g_name_mode = 1;
                        g_list_top = 0;
                        g_name_col = 1;
                        g_name_row = 0;
                        g_list_code--;
                        g_sprite_attr[44] |= ATTR_HIDE;
                        g_sprite_attr[45] |= ATTR_HIDE;
                        g_sprite_attr[10] |= ATTR_HIDE;
                        g_sprite_attr[11] &= ~ATTR_HIDE;
                        NameListFill(g_list_code, g_name_list_pos);
                    }
                }
                break;
            case 0:
                if (g_name_row < 4 && (g_pad_trig & PAD_CIRCLE)) {
                    if (g_name_field == 2 && g_name_row == 3) {
                        NamePlaySe(2);
                    } else {
                        NamePlaySe(4);
                        NameSetPage(g_name_row);
                    }
                } else if (g_name_row == 5) {
                    NamePlaySe(4);
                    g_name_field = (g_name_field + 1) % 3;
                    for (i = 0; i < 3; i++) {
                        if (i == g_name_field) {
                            g_sprite_attr[12 + (i + 1) % 3] &= ~ATTR_HIDE;
                        } else {
                            g_sprite_attr[12 + (i + 1) % 3] |= ATTR_HIDE;
                        }
                    }
                    if (g_name_field == 2 && g_name_page == 3) {
                        NameSetPage(0);
                    }
                }
                break;
            case 11:
                if (g_name_row < 2) {
                    if (g_name_row == 0) {
                        if (CUR > 0) {
                            NamePlaySe(1);
                            CUR--;
                        } else if (g_pad_trig & PAD_CIRCLE) {
                            NamePlaySe(1);
                        }
                    } else if (CUR < g_name_len[g_name_field] - 1) {
                        NamePlaySe(1);
                        CUR++;
                    } else if (g_pad_trig & PAD_CIRCLE) {
                        NamePlaySe(1);
                    }
                } else if (g_name_row == 2) {
                    if (CUR != 0) {
                        NamePlaySe(3);
                        if (CARET_CHAR == 0) {
                            CUR--;
                        }
                    } else if (g_name_text[g_name_field][0] != 0 || (g_pad_trig & PAD_CIRCLE)) {
                        NamePlaySe(3);
                    }
                    CARET_CHAR = 0;
                    NameDrawCursor();
                } else if (g_name_row == 3) {
                    if (g_pad_trig & PAD_CIRCLE) {
                        NamePlaySe(3);
                        for (i = NAME_CELLS - 1; i >= 0; i--) {
                            g_name_text[g_name_field][i] = 0;
                        }
                        for (i = 0; i < g_name_len[g_name_field]; i++) {
                            ExpandGlyph(g_name_text[g_name_field][i], &g_glyph_cell[i * 2],
                                        g_name_len[g_name_field] * 2);
                        }
                        UploadImage(g_name_x[g_name_field] + 0x200, g_name_y[g_name_field],
                                    g_name_len[g_name_field] * 4, 0x10, g_glyph_cell);
                        CUR = 0;
                    }
                } else if (g_pad_trig & PAD_CIRCLE) {
                    if (NameEntryComplete()) {
                        NamePlaySe(1);
                        if (NameConfirm() == 0) {
                            NamePlaySe(1);
                            SsSeqSetDecrescendo(g_seq_handles[0], 0x7F, 0x78);
                            return 0;
                        }
                    }
                    NamePlaySe(2);
                }
                break;
            }
        } else if (g_pad_held & PAD_CROSS) {
            if (CUR != 0) {
                NamePlaySe(2);
                if (CARET_CHAR == 0) {
                    CUR--;
                }
            } else if (g_name_text[g_name_field][0] != 0 || (g_pad_trig & PAD_CROSS)) {
                NamePlaySe(2);
            }
            CARET_CHAR = 0;
            NameDrawCursor();
        } else if ((g_pad_held & PAD_TRI) && g_name_col != 0 && g_name_col != 11) {
            NamePlaySe(1);
            g_name_col = 0;
            g_name_row = 5;
        } else if (g_pad_held & PAD_SELECT) {
            NamePlaySe(4);
            page = (g_name_page + 1) & 3;
            if (g_name_field == 2 && page == 3) {
                page = 0;
            }
            NameSetPage(page);
        } else if (g_pad_trig & PAD_START) {
            NamePlaySe(1);
            g_name_col = 11;
            g_name_row = 5;
        }
        break;

    case 1:
        if (g_name_repeat != 0) {
            return 1;
        }
        if (g_pad_held & PAD_UP) {
            NamePlaySe(0);
            if (g_name_row == 0 && g_name_col != 0 && g_name_col != 11) {
                NameListScroll(1);
                if (--g_list_top < 0) {
                    g_list_top = 6;
                }
                g_bg_frame.scrolly -= 0x10;
                g_bg_fields.scrolly -= 0x10;
                g_list_code = g_list_rows[g_list_top][0];
                g_name_list_pos = g_list_rows[g_list_top][1];
                NameListPreview();
            } else if (--g_name_row < 0) {
                g_name_row = 5;
            } else if (g_name_row == 4 && (g_name_col == 0 || g_name_col == 11)) {
                g_name_row = 3;
            }
        } else if (g_pad_held & PAD_DOWN) {
            NamePlaySe(0);
            if (g_name_row == 5 && g_name_col != 0 && g_name_col != 11) {
                NameListScroll(0);
                if (++g_list_top >= LIST_SLOTS) {
                    g_list_top = 0;
                }
                g_bg_frame.scrolly += 0x10;
                g_bg_fields.scrolly += 0x10;
                g_list_code = g_list_rows[g_list_top][0];
                g_name_list_pos = g_list_rows[g_list_top][1];
                NameListPreview();
            } else if (++g_name_row >= 6) {
                g_name_row = 0;
            } else if (g_name_row == 4 && (g_name_col == 0 || g_name_col == 11)) {
                g_name_row = 5;
            }
        } else if (g_pad_held & PAD_LEFT) {
            NamePlaySe(0);
            if (--g_name_col < 0) {
                g_name_col = 11;
            }
            if (g_name_row == 4 && g_name_col == 0) {
                g_name_row = 5;
            }
        } else if (g_pad_held & PAD_RIGHT) {
            NamePlaySe(0);
            if (++g_name_col >= 12) {
                g_name_col = 0;
            }
            if (g_name_row == 4 && g_name_col == 11) {
                g_name_row = 5;
            }
        } else if (g_pad_held & PAD_L1) {
            if (CUR > 0) {
                NamePlaySe(0);
                CUR--;
            } else if (g_pad_trig & PAD_L1) {
                NamePlaySe(0);
            }
        } else if (g_pad_held & PAD_R1) {
            if (CUR < g_name_len[g_name_field] - 1) {
                NamePlaySe(0);
                CUR++;
            } else if (g_pad_trig & PAD_R1) {
                NamePlaySe(0);
            }
        } else if (g_pad_held & PAD_L2) {
            NamePlaySe(0);
            top = g_list_top;
            g_list_code = g_list_rows[(top + g_name_row) % LIST_SLOTS][0];
            if (g_name_row == 0 && g_list_rows[top % LIST_SLOTS][1] == 0) {
                if (--g_list_code < 0) {
                    g_list_code = KANJI_GROUPS - 1;
                }
            }
            g_name_col = 1;
            g_name_row = 0;
            g_list_top = 0;
            g_name_list_pos = 0;
            NameListFill(g_list_code, 0);
            NameListPreview();
        } else if (g_pad_held & PAD_R2) {
            NamePlaySe(0);
            top = g_list_top;
            g_list_code = g_list_rows[(top + g_name_row) % LIST_SLOTS][0];
            if (g_name_row == 0 && g_list_rows[top % LIST_SLOTS][1] == 0) {
                if (++g_list_code >= KANJI_GROUPS) {
                    g_list_code = 0;
                }
            }
            g_name_col = 1;
            g_name_row = 0;
            g_list_top = 0;
            g_name_list_pos = 0;
            NameListFill(g_list_code, 0);
            NameListPreview();
        } else if (g_pad_held & PAD_CIRCLE) {
            if (g_name_col == 0) {
                if (g_name_row < 4 && (g_pad_trig & PAD_CIRCLE)) {
                    NamePlaySe(4);
                    NameListClose();
                    NameSetPage(g_name_row);
                } else if (g_name_row == 5) {
                    NamePlaySe(4);
                    g_name_field = (g_name_field + 1) % 3;
                    for (i = 0; i < 3; i++) {
                        if (i == g_name_field) {
                            g_sprite_attr[12 + (i + 1) % 3] &= ~ATTR_HIDE;
                        } else {
                            g_sprite_attr[12 + (i + 1) % 3] |= ATTR_HIDE;
                        }
                    }
                    if (g_name_field == 2 && g_name_page == 3) {
                        NameListClose();
                        NameSetPage(0);
                    }
                }
            } else if (g_name_col == 11) {
                if (g_name_row < 2) {
                    if (g_name_row == 0) {
                        if (CUR > 0) {
                            NamePlaySe(1);
                            CUR--;
                        } else if (g_pad_trig & PAD_CIRCLE) {
                            NamePlaySe(1);
                        }
                    } else if (CUR < g_name_len[g_name_field] - 1) {
                        NamePlaySe(1);
                        CUR++;
                    } else if (g_pad_trig & PAD_CIRCLE) {
                        NamePlaySe(1);
                    }
                } else if (g_name_row == 2) {
                    if (CUR != 0) {
                        NamePlaySe(3);
                        if (CARET_CHAR == 0) {
                            CUR--;
                        }
                    } else if (g_name_text[g_name_field][0] != 0 || (g_pad_trig & PAD_CIRCLE)) {
                        NamePlaySe(3);
                    }
                    CARET_CHAR = 0;
                    NameDrawCursor();
                } else if (g_name_row == 3) {
                    if (g_pad_trig & PAD_CIRCLE) {
                        NamePlaySe(3);
                        for (i = NAME_CELLS - 1; i >= 0; i--) {
                            g_name_text[g_name_field][i] = 0;
                        }
                        for (i = 0; i < g_name_len[g_name_field]; i++) {
                            ExpandGlyph(g_name_text[g_name_field][i], &g_glyph_cell[i * 2],
                                        g_name_len[g_name_field] * 2);
                        }
                        UploadImage(g_name_x[g_name_field] + 0x200, g_name_y[g_name_field],
                                    g_name_len[g_name_field] * 4, 0x10, g_glyph_cell);
                        CUR = 0;
                    }
                } else if (g_pad_trig & PAD_CIRCLE) {
                    if (NameEntryComplete()) {
                        NamePlaySe(1);
                        if (NameConfirm() == 0) {
                            NamePlaySe(1);
                            return 0;
                        }
                    }
                    NamePlaySe(2);
                }
            } else {
                slot = (g_list_top + g_name_row) % LIST_SLOTS;
                CARET_CHAR = g_kanji_rows[g_list_rows[slot][0]][g_list_rows[slot][1]][g_name_col - 1];
                NameDrawCursor();
                if (CUR != g_name_len[g_name_field] - 1) {
                    NamePlaySe(1);
                    CUR++;
                } else if (g_pad_trig & PAD_CIRCLE) {
                    NamePlaySe(1);
                }
            }
        } else if (g_pad_held & PAD_CROSS) {
            if (CUR != 0) {
                NamePlaySe(2);
                if (CARET_CHAR == 0) {
                    CUR--;
                }
            } else if (g_name_text[g_name_field][0] != 0 || (g_pad_trig & PAD_CROSS)) {
                NamePlaySe(2);
            }
            CARET_CHAR = 0;
            NameDrawCursor();
        } else if ((g_pad_held & PAD_TRI) && g_name_col != 0 && g_name_col != 11) {
            NamePlaySe(1);
            g_name_col = 0;
            g_name_row = 5;
        } else if (g_pad_held & PAD_SELECT) {
            NamePlaySe(4);
            page = (g_name_page + 1) & 3;
            if (g_name_field == 2 && page == 3) {
                page = 0;
            }
            NameListClose();
            NameSetPage(page);
        } else if (g_pad_trig & PAD_START) {
            NamePlaySe(1);
            g_name_col = 11;
            g_name_row = 5;
        }
        break;
    }

    if (g_name_mode == 0) {
        if (g_name_col == 0) {
            g_caret_x = 0x10;
        } else if (g_name_col == 11) {
            g_caret_x = 0x100;
        } else {
            g_caret_x = (g_name_col - 1) * 16 + 0x48;
            if (g_name_col >= 6) {
                g_caret_x = (g_name_col - 1) * 16 + 0x58;
            }
        }
    } else if (g_name_col == 0) {
        g_caret_x = 0x10;
    } else if (g_name_col == 11) {
        g_caret_x = 0x100;
    } else {
        g_caret_x = (g_name_col - 1) * 16 + 0x48;
        if (g_name_col > 0) {
            g_caret_x = (g_name_col - 1) * 16 + 0x58;
        }
    }
    g_caret_y = g_name_row * 16 + 0x78;
    NameCaretPlace();
    return 1;
}
#endif

/* Plays sound effect `se`; the handles after the first are the effects. */
void NamePlaySe(int se)
{
    SsPlayBack(g_seq_handles[se + 1], 0, 1);
}
