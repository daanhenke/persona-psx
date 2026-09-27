/* cc1flags: -O0 -G8 */
/* Persona 1 (JP) - OPEN.EXE, the title @ 0x80081018
 *
 * The title and its demo screens: sprite set-up, the font, the fades and
 * the draw loops. Built without optimisation like the rest of this
 * executable.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <libetc.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <libsnd.h>
#include <persona/open/open.h>

/* Defined here: the unit reaches them gp-relative. */
int g_pad_now;
int g_pad_old;
int g_pad_trig;
int g_text_len;
int g_msg_len;

/* The font's sprites run 34 to a text row. */
#define FONT_CELL(row, col) ((row) * 34 + (col))

INCLUDE_ASM("open/nonmatchings/title", func_80081018);

void OpenDrawFrame(void)
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
    if (D_800B40F8) {
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

INCLUDE_ASM("open/nonmatchings/title", func_80082E00);

void func_80084288(void)
{
    D_800B410C = 2;
    func_80085798();
    g_pad_now = -1;
    g_pad_trig = 0;
}

void func_800842D0(int arg0)
{
    int i;

    D_800B410C = 0;
    for (i = 0; i < 0x103; i++) {
        g_sprite_flags[i + 10] = 0;
    }
    func_80084B40(arg0);
    OpenFontLoadText(12, 2, 0x40, 0x54, D_800A0C10);
    g_pad_now = -1;
    g_pad_trig = 0;
}

INCLUDE_ASM("open/nonmatchings/title", func_800843A0);

void func_80084B40(int alt)
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

INCLUDE_ASM("open/nonmatchings/title", func_80084E64);

void func_80085798(void)
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

void func_8008615C(int kind, int port, int arg2)
{
    OpenPlaySeq(3);
    switch (kind) {
    case 1:
        OpenMessageOpen(12, 1, 0x40, 0x5C, D_800A0C5C);
        break;
    case 2:
        OpenMessageOpen(13, 1, 0x38, 0x58, D_800A0C40);
        break;
    case 4:
        OpenMessageOpen(13, 2, 0x38, 0x54, D_800A0C74);
        break;
    }
    func_800843A0();
    CardPollPorts(0, 0);
    for (;;) {
        if (CardPollPorts(port == 0 ? 0x81 : 0, port ? 0x81 : 0)) {
            func_800842D0(arg2);
            break;
        }
        func_800843A0();
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

void OpenFontRenderGlyph(u_short code)
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
