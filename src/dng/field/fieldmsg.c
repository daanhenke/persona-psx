/* Persona 1 (JP) - the field's message box.  DNG only.
 *   0x80074AB8 FieldMsgPutGlyph
 *   0x80074BC0 FieldMsgClear
 *   0x80074C3C FieldMsgNewLine
 *   0x80074D64 FieldMsgClearLine
 *   0x80074DD8 FieldMsgSetWindow
 *   0x80074EB4 FieldMsgTint
 *   0x80074F28 FieldMsgPrintBytes
 *   0x80074FA4 FieldMsgPrint
 *   0x80075054 FieldFindMember
 *   0x800750B8 FieldMsgPrintCodes
 *   0x8007518C FieldMsgSetStyle
 *
 * Text is drawn into a strip of VRAM beside the screen, three lines of
 * fifteen glyphs, and shown through a primitive list in the pack whose cells
 * carry the text colour's palette. A fourth line scrolls the window up.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <persona/common/char.h>
#include <persona/common/status.h>
#include <persona/dng/field.h>

#define MSG_LINES   3
#define MSG_COLS    15
#define MSG_VRAM_X  320
#define MSG_VRAM_Y  24
#define MSG_LINE_H  16

/* The message box's cells in the pack. */
#define MSG_CELLS ((u_char *)PACK_BASE + g_pack_msg_tab[3])

/* Draws glyph `n` at the cursor and moves it on, starting a new line when
   the one it is on is full. */
void FieldMsgPutGlyph(u_short n)
{
    if (g_msg_col >= MSG_COLS) {
        FieldMsgNewLine();
    }
    FieldDecodeGlyph(n);
    FieldMsgTint(MSG_CELLS, g_msg_color, g_msg_col, g_msg_line, 1);
    g_msg_rect.x = g_msg_col * 4 + MSG_VRAM_X;
    g_msg_rect.y = g_msg_line % MSG_LINES * MSG_LINE_H + MSG_VRAM_Y;
    LoadImage(&g_msg_rect, (u_long *)g_scene->glyph);
    g_msg_col++;
}

/* Empties the box and recolours all of it. */
void FieldMsgClear(void)
{
    RECT r;

    r.x = MSG_VRAM_X;
    r.y = MSG_VRAM_Y;
    r.w = MSG_COLS * 4;
    r.h = MSG_LINES * MSG_LINE_H;
    ClearImage(&r, 0, 0, 0);
    FieldMsgTint(MSG_CELLS, g_msg_color, 0, 0, MSG_LINES * MSG_COLS);
}

/* Moves the cursor to the start of the next line. From the fourth line on
   the window scrolls up a line over four frames and the line it reuses is
   wiped. */
void FieldMsgNewLine(void)
{
    int i;

    g_msg_col = 0;
    if (++g_msg_line >= MSG_LINES) {
        for (i = 0; i < 4; i++) {
            g_scene->layers[LAYER_MSG].bg.y -= 4;
            FieldFrame();
        }
        FieldMsgClearLine(g_msg_line % MSG_LINES);
        g_scene->layers[LAYER_MSG].bg.y += MSG_LINE_H;
        g_scene->layers[LAYER_MSG].bg.scrolly += MSG_LINE_H;
    }
    FieldMsgTint(MSG_CELLS, g_msg_color, 0, g_msg_line, MSG_COLS);
}

/* Wipes one line of the box's VRAM. */
void FieldMsgClearLine(int line)
{
    RECT r;

    r.x = MSG_VRAM_X;
    r.y = line % MSG_LINES * MSG_LINE_H + MSG_VRAM_Y;
    r.w = MSG_COLS * 4;
    r.h = MSG_LINE_H;
    ClearImage(&r, 0, 0, 0);
}

/* Sets the selection window up: the default kind when `n` is 0, otherwise
   kind n's entry of the window table. */
void FieldMsgSetWindow(int n)
{
    g_scene->win.y = 0x1E0;
    g_scene->win.rows = 1;
    if (n == 0) {
        g_scene->win.kind = 2;
        g_scene->win.first = 0x20;
        g_scene->win.count = 11;
        g_scene->win.src = g_win_default;
    } else {
        g_scene->win.kind = 1;
        g_scene->win.first = n + 0x1F;
        g_scene->win.count = 1;
        g_scene->win.src = &g_win_kinds[n];
    }
}

/* Gives `count` cells from (col, line) the palette of text colour `color`. */
void FieldMsgTint(u_char *cells, u_char color, int col, int line, int count)
{
    cells += ((line % MSG_LINES) * MSG_COLS + col) * 8 + 0xA;
    for (; count > 0; count--) {
        *(u_short *)cells = g_msg_cluts[color];
        cells += 8;
    }
}

/* Text ends at MSG_END; a glyph code with its top bit set takes the next
   byte as its low half. */
#define MSG_END      0xFF
#define MSG_CODE_END 1
#define MSG_CODE_NL  3

/* Prints up to `n` single-byte glyphs, a frame each. */
void FieldMsgPrintBytes(u_char *s, u_char n)
{
    int    i;
    u_char c;

    for (i = 0; i < n; i++) {
        c = *s;
        if (c == MSG_END) {
            break;
        }
        s++;
        FieldMsgPutGlyph(c);
        FieldFrame();
    }
}

/* Prints up to `n` glyph codes, a frame each. Old-style, like the rest of
   the printers: n is narrowed at every test. */
/* 87.8%: the double read is the int c against the QImode top-bit test, as
   in FieldMsgPrintCodes. What is left is MSG_END: the image's loop.c hoists
   the 0xFF into s3, and here the move is "not desirable" at every loop size
   tried (28-30 insns), so the compare loads it each pass. */
#ifdef NON_MATCHING
void FieldMsgPrint(s, n)
    u_char *s;
    u_char  n;
{
    int    i;
    int    c;
    int    g;

    for (i = 0; i < n; i++) {
        c = *s;
        if (c == MSG_END) {
            break;
        }
        if (!(*s & 0x80)) {
            s++;
            FieldMsgPutGlyph(c);
        } else {
            g = ((*s & 0x7F) << 8) | s[1];
            s += 2;
            FieldMsgPutGlyph(g);
        }
        FieldFrame();
    }
}
#else
INCLUDE_ASM("dng/nonmatchings/field/fieldmsg", FieldMsgPrint);
#endif

/* The party slot holding the character whose record key is `key` + 1, or
   -1. */
int FieldFindMember(u_char key)
{
    u_char *p;
    int     i;

    p = g_party;
    for (i = 0; i < 5; i++, p++) {
        if (*p != 0xFF && g_chars[*p].key == key + 1) {
            return *p;
        }
    }
    return -1;
}

/* Prints up to `n` glyph codes or control codes, a frame each. Returns
   nothing, but is declared implicit int: v0 stays live at the exit. The
   glyph byte is read into an int, which the QImode top-bit test does not
   share, so the image reads it twice. */
FieldMsgPrintCodes(s, n)
    u_char *s;
    u_char  n;
{
    int     i;
    int     c;
    int     g;
    int     k;

    for (i = 0; i < n; i++) {
        c = *s;
        if (c == MSG_END) {
            k = *++s;
            if (k == MSG_CODE_END) {
                break;
            }
            s++;
            if (k == MSG_CODE_NL) {
                FieldMsgNewLine();
            }
        } else {
            if (!(*s & 0x80)) {
                s++;
                FieldMsgPutGlyph(c);
            } else {
                g = ((*s & 0x7F) << 8) | s[1];
                s += 2;
                FieldMsgPutGlyph(g);
            }
        }
        FieldFrame();
    }
}

/* A style byte: the low five bits one setting, the top three an index into
   a table for the other. */
void FieldMsgSetStyle(u_int style)
{
    D_8009FAE4 = g_msg_styles[(style >> 5) & 7];
    D_8009FAE0 = style & 0x1F;
}
