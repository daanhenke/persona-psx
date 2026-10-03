/* Persona 1 (JP) - the text windows.  S2D.
 *   0x8009A2A8 S2dWinInit      0x8009A368 (the draw, asm)
 *   0x8009A658 S2dWinSetArea   0x8009A714 S2dWinCheckFull
 *   0x8009A774 S2dReadChar     0x8009A7B8 S2dCopyName
 *   0x8009A804 S2dCopyChars    0x8009A890 (asm)
 *   0x8009AB20 S2dWinScroll    0x8009AC04 S2dWinState
 *   0x8009AC18 (the script step, asm)  0x8009B428 S2dWinSetParam
 *
 * A window runs a script that lays text into it; characters are one byte
 * below 0x80 and two above, and a string ends in 0xFF.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <libgte.h>
#include <libgpu.h>
#include <persona/s2d/textwin.h>

extern void S2dIndicatorBar(void);
/* Declared without a prototype: the scroll hands its height over
   unnarrowed. */
extern void VramQueueClear();

/* Seven settings the window scripts read. */
extern int D_800B5EF8[7];

void S2dWinInit(mode, w, script, a, b, c, d, cols, rows, x, y)
    short   mode;
    S2dWin *w;
    int     script;
    short   a, b, c, d;
    int     cols;
    short   rows, x, y;
{
    w->mode = mode;
    w->unk06 = 0;
    w->count = 0;
    w->flags = 0;
    w->unk04 = 0;
    w->script = script;
    w->unk1C4 = a;
    w->unk1C6 = b;
    w->unk1C8 = c;
    w->unk1CA = d;
    w->x = x;
    w->y = y;
    w->x0 = x;
    w->y0 = y;
    w->cols = cols;
    w->rows = rows;
    if (mode & WIN_NO_XOFS) {
        w->ofs_x = 0;
    } else {
        w->ofs_x = -(short)cols * 8;
    }
    w->ofs_y = -rows * 8;
    w->trans.vz = 0x280;
    w->trans.vx = 0;
    w->trans.vy = 0;
    w->rot.vx = 0;
    w->rot.vy = 0;
    w->rot.vz = 0;
    w->scale.vx = ONE;
    w->scale.vy = ONE;
    w->scale.vz = ONE;
}

INCLUDE_ASM("s2d/nonmatchings/ui/textwin", func_8009A368);

/* The window's two draw areas for one buffer: the text's rows, and the
   whole screen. */
void S2dWinSetArea(S2dWin *w, int side, int x, int y)
{
    RECT text;
    RECT all;

    text.x = x - 0x100;
    text.y = y + (w->y0 + w->ofs_y);
    text.w = 0x200;
    text.h = w->rows * 16;
    SetDrawArea(&w->text_area[side], &text);
    all.x = x - 0x100;
    all.y = y - 0x78;
    all.w = 0x200;
    all.h = 0xF0;
    SetDrawArea(&w->area[side], &all);
}

/* A window whose every place is filled waits for the pad. */
void S2dWinCheckFull(S2dWin *w)
{
    if (w->count == w->cols * w->rows) {
        S2dIndicatorBar();
        w->flags = (w->flags & ~0x10) | 0x64;
    }
}

/* One character: a byte below 0x80, or two with the top bit cleared. Returns
   where its last byte is. */
u_char *S2dReadChar(u_char *s, u_short *c)
{
    u_short code;

    if (*s < 0x80) {
        code = *s;
    } else {
        u_char t;

        t = *s++;
        code = *s | ((t & 0x7F) << 8);
    }
    *c = code;
    return s;
}

/* A name of up to ten single-byte characters, as halfwords. */
void S2dCopyName(u_char *s, u_short *out)
{
    int i;

    i = 0;
    while (*s != 0xFF) {
        if (i == 10) {
            break;
        }
        *out++ = *s++;
        i++;
    }
    *out = TEXT_END;
}

/* Up to five characters of either width, as halfwords. */
void S2dCopyChars(u_char *s, u_short *out)
{
    u_short c;
    int     i;

    i = 0;
    while (*s != 0xFF) {
        if (i == 5) {
            break;
        }
        i++;
        s = S2dReadChar(s, &c);
        s++;
        *out++ = c;
    }
    *out = TEXT_END;
}

INCLUDE_ASM("s2d/nonmatchings/ui/textwin", func_8009A890);

/* One step of the window's scroll: four lines up every frame, and after a
   whole window the text starts again from the top with its glyphs
   cleared. Returns how far it has scrolled. */
u_short S2dWinScroll(S2dWin *w)
{
    if (--w->scroll_wait == 0) {
        w->y -= 4;
        if (++w->scroll >= w->rows * 4) {
            w->y = w->y0;
            if (!(w->mode & 1)) {
                w->unk06 = 0;
                w->count = 0;
            }
            w->scroll = 0;
            VramQueueClear(w->unk1C4, w->unk1C6, 0x40,
                           ((w->cols * w->rows) / 16 + 1) * 16, 0, 0, 0);
        }
        w->scroll_wait = 1;
    }
    return w->scroll;
}

u_long S2dWinState(S2dWin *w)
{
    return w->flags | (w->unk04 << 16);
}

INCLUDE_ASM("s2d/nonmatchings/ui/textwin", func_8009AC18);

void S2dWinSetParam(int which, int v)
{
    int *p = D_800B5EF8;

    switch (which) {
    case 0:
        p[0] = v;
        break;
    case 1:
        p[1] = v;
        break;
    case 2:
        p[3] = v;
        break;
    case 3:
        p[4] = v;
        break;
    case 4:
        p[6] = v;
        break;
    case 5:
        p[5] = v;
        break;
    case 6:
        p[2] = v;
        break;
    }
}
