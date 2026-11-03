/* Persona 1 (JP) - the text windows.  S2D.
 *   0x8009A2A8 S2dWinInit      0x8009A368 (the draw, asm)
 *   0x8009A658 S2dWinSetArea   0x8009A714 S2dWinCheckFull
 *   0x8009A774 S2dReadChar     0x8009A7B8 S2dCopyName
 *   0x8009A804 S2dCopyChars    0x8009A890 (place and upload glyph)
 *   0x8009AB20 S2dWinScroll    0x8009AC04 S2dWinState
 *   0x8009AC18 (the script step)  0x8009B428 S2dWinSetParam
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

typedef struct {
    u_char  u, v;
    u_short clut;
    short   x, y;
} S2dWinGlyph;

extern int D_800B85BC;
extern int D_800AA610;
extern void BtlExpandGlyph(int code, u_int *dst, int font);
/* This caller passes the coordinate sums without narrowing them. */
extern void VramQueueLoad();

/* Place a glyph in the window and queue its 4-bit texture upload.
   The upload buffer wraps after 45 glyphs of 128 bytes each. */
void func_8009A890(S2dWin *w, u_short ch)
{
    S2dWinGlyph *g;

    g = (S2dWinGlyph *)(w->unk1C + w->unk06 * 8);
    g->x = (w->count % w->cols) * 16;
    g->y = (w->count / w->cols) * 16;
    g->u = (short)(w->unk1C4 % 64) * 4 + (short)(w->count % 15) * 16;
    g->v = w->unk1C6 % 256 + (short)(w->count / 15) * 16;
    g->clut = GetClut(w->unk1C8, w->unk1CA + w->unk10);
    BtlExpandGlyph(ch, (u_int *)(D_800B85BC + D_800AA610), 0x801E0000);
    VramQueueLoad(D_800B85BC + D_800AA610,
                  w->unk1C4 + (short)(w->count % 15) * 4,
                  w->unk1C6 + (short)(w->count / 15) * 16, 4, 16);
    D_800AA610 += 128;
    if (D_800AA610 >= 0x1680) {
        D_800AA610 = 0;
    }
    w->unk06++;
    w->count++;
}

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

extern int D_800AA618;
extern u_char D_800B8640[], g_member_names[][20], g_insert8_names[][40];
extern void BtlFormatDecimal();
extern void S2dIndicatorClear(void);
extern int func_8008E158(int);

#define BYTE (*(u_char *)w->script)
#define TEXT ((short *)w->unk1DC)

/* Advance the text script through control codes, inserted strings and waits. */
int func_8009AC18(S2dWin *w)
{
    u_short ch;
    int first, pressed;
    if (!(w->flags & 1)) {
        w->unk06 = 0;
        if (!(w->flags & 2))
            w->unk10 = 0;
        w->scroll = 0;
        w->unk16 = 0;
        w->scroll_wait = 1;
        w->count = 0;
        w->flags = (w->flags | 0x8003) & ~0x100;
    }
    if (!(w->flags & 4)) {
        if (!(w->flags & 0x100)) {
        again:
            first = BYTE;
            if (first == 0xFF) {
                w->script++;
                switch (BYTE) {
                case 1:
                    if (w->mode & 1)
                        w->flags = 0;
                    else
                        w->flags |= 0xA4;
                    break;
                case 0xFF:
                    w->script++;
                    w->unk16 = BYTE;
                    w->script++;
                    goto again;
                case 5:
                    w->script++;
                    w->unk14 = BYTE;
                    w->unk14 |= *(u_char *)(++w->script) << 8;
                    w->flags |= 0x14;
                    break;
                case 3:
                    w->count += w->cols - w->count % w->cols;
                    S2dWinCheckFull(w);
                    break;
                case 4:
                    w->count = w->cols * w->rows;
                    S2dWinCheckFull(w);
                    break;
                case 2:
                    S2dIndicatorBar();
                    w->flags |= 0x24;
                    break;
                case 0xFE:
                    w->script++;
                    w->unk10 = BYTE;
                    w->script++;
                    goto again;
                case 0xF8:
                    BtlFormatDecimal(D_800B5EF8[0], D_800B8640, 0);
                    for (ch = 0; D_800B8640[ch] != 0xFF; ch++)
                        TEXT[ch] = D_800B8640[ch] + 0xC0;
                    TEXT[ch] = 0x1FF;
                    w->unk08 = (int)TEXT;
                    w->flags |= 0x100;
                    break;
                case 0xF3:
                    S2dCopyName(D_800B5EF8[3], TEXT);
                    w->unk08 = (int)TEXT;
                    w->flags |= 0x100;
                    break;
                case 0xF4:
                    S2dCopyName(D_800B5EF8[4], TEXT);
                    w->unk08 = (int)TEXT;
                    w->flags |= 0x100;
                    break;
                case 0xF2:
                    S2dCopyName(D_800B5EF8[5], TEXT);
                    w->unk08 = (int)TEXT;
                    w->flags |= 0x100;
                    break;
                case 7:
                    w->script++;
                    S2dCopyChars(
                        ((u_char *)((BYTE - 1) * 20 + (u_long)g_member_names)) +
                            10,
                        TEXT);
                    w->unk08 = (int)TEXT;
                    w->flags |= 0x100;
                    break;
                case 15:
                    w->script++;
                    S2dCopyChars(
                        ((u_char *)((BYTE - 1) * 20 + (u_long)g_member_names)),
                        TEXT);
                    w->unk08 = (int)TEXT;
                    w->flags |= 0x100;
                    break;
                case 8:
                    w->script++;
                    S2dCopyName(g_insert8_names[BYTE], TEXT);
                    w->unk08 = (int)TEXT;
                    w->flags |= 0x100;
                    break;
                case 16:
                    w->script++;
                    S2dCopyChars(
                        ((u_char *)((BYTE - 1) * 20 + (u_long)g_member_names)),
                        TEXT);
                    for (ch = 0; ch < 24; ch++)
                        if (TEXT[ch] == (short)0xFF01)
                            break;
                    TEXT[ch] = 0;
                    ch++;
                    S2dCopyChars(
                        ((u_char *)((BYTE - 1) * 20 + (u_long)g_member_names)) +
                            10,
                        TEXT + ch);
                    w->unk08 = (int)TEXT;
                    w->flags |= 0x100;
                    break;
                case 0xFA:
                    S2dCopyName(D_800B5EF8[1], TEXT);
                    w->unk08 = (int)TEXT;
                    w->flags |= 0x100;
                    break;
                case 0xF9:
                    S2dCopyName(D_800B5EF8[6], TEXT);
                    w->unk08 = (int)TEXT;
                    w->flags |= 0x100;
                    break;
                case 0xF0:
                    S2dCopyChars(D_800B5EF8[2], TEXT);
                    w->unk08 = (int)TEXT;
                    w->flags |= 0x100;
                    break;
                case 0xFD:
                    w->script++;
                    w->unk04 = (BYTE << 8) | 1;
                    w->script++;
                    w->flags |= 0x204;
                    break;
                case 0xF1:
                    w->unk04 = 2;
                    w->flags |= 0x224;
                    break;
                case 0xEF:
                    w->unk04 = 3;
                    w->flags |= 0x204;
                    break;
                default:
                    w->script++;
                    break;
                }
            } else {
                w->script = (int)S2dReadChar((u_char *)w->script, &ch);
                w->flags |= 8;
            }
            w->script++;
        }
        if (w->flags & 0x100) {
            if (*(u_short *)w->unk08 != 0xFF01) {
                ch = *((u_short *)w->unk08)++;
                w->flags |= 8;
            } else
                w->flags &= ~0x100;
        }
    }
    if ((*(u_int *)w & 0x600000) == 0x400000) {
        if (S2dWinScroll(w) == 0) {
            w->count = 0;
            w->unk14 = 11;
            w->flags = (w->flags & ~0x41) | 0x10;
        }
    }
    if (w->flags & 8) {
        func_8009A890(w, ch);
        w->flags = (w->flags & ~8) | 0x14;
        w->unk14 = w->unk16 + 1;
        S2dWinCheckFull(w);
    }
    if (w->flags & 0x10) {
        if (--w->unk14 == 0)
            w->flags &= ~0x14;
    }
    pressed = func_8008E158(1);
    D_800AA618 = 0;
    if (w->flags & 0x20) {
        if (pressed) {
            S2dIndicatorClear();
            w->flags &= ~0x20;
            if (!(w->flags & 0x40))
                w->flags &= ~4;
            if (w->flags & 0x80)
                w->flags = 0;
        } else
            D_800AA618 = 1;
    }
    return w->flags;
}

#undef BYTE
#undef TEXT

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
