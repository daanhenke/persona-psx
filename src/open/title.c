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
int D_800B4094;
int D_800B409C;

/* The font's sprites run 34 to a text row. */
#define FONT_CELL(row, col) ((row) * 34 + (col))

INCLUDE_ASM("open/nonmatchings/title", func_80081018);

INCLUDE_ASM("open/nonmatchings/title", func_800824AC);

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
    D_800B4094 = -1;
    D_800B409C = 0;
}

void func_800842D0(int arg0)
{
    int i;

    D_800B410C = 0;
    for (i = 0; i < 0x103; i++) {
        D_8011F390[i] = 0;
    }
    func_80084B40(arg0);
    func_80085D80(12, 2, 0x40, 0x54, D_800A0A90 + 0x180);
    D_800B4094 = -1;
    D_800B409C = 0;
}

INCLUDE_ASM("open/nonmatchings/title", func_800843A0);

INCLUDE_ASM("open/nonmatchings/title", func_80084B40);

INCLUDE_ASM("open/nonmatchings/title", func_80084E64);

INCLUDE_ASM("open/nonmatchings/title", func_80085798);

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

INCLUDE_ASM("open/nonmatchings/title", func_80085D80);

INCLUDE_ASM("open/nonmatchings/title", func_8008615C);

INCLUDE_ASM("open/nonmatchings/title", func_80086314);

INCLUDE_ASM("open/nonmatchings/title", func_8008678C);
