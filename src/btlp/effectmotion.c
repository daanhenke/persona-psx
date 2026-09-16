/* Persona 1 (JP) - how an effect's box opens and shuts.  BTLP only.
 *   0x8007783C BtlEffectMotionHold      0x80077844 BtlEffectMotionGrow
 *   0x800778F0 BtlEffectMotionUnroll    0x8007792C BtlEffectMotionRoll
 *   0x80077968 BtlEffectMotionOpenNow   0x800779A8 BtlEffectMotionShutNow
 *   0x800779DC BtlEffectMotionShrink    0x80077A8C BtlEffectMotionCollapse
 *   0x80077ACC BtlDrawGlyphs            0x80077EF4 BtlEffectDrawRow
 *   0x80077F30 BtlFormatHexGlyphs
 *
 * The draw pass takes the low nibble of BtlEffect.kind, picks one of the eight
 * out of a table, and calls it with the record. Each one moves the box's two
 * scales a frame further and answers whether the effect is still running; the
 * pass clears the slot for one that answers no.
 *
 * They come in pairs, one opening and one shutting: Grow and Shrink work in
 * thirds so the movement eases off, Unroll and Roll in fixed steps, and
 * OpenNow and ShutNow do it in a single frame. Hold is the resting one - the
 * box is already the size it wants, so it does nothing and says so. Getting
 * there is what the other handlers write into the nibble as they finish.
 *
 * The cursor is put away while a box is moving and brought back once it has
 * settled, which is why the handlers that end open show it and the ones that
 * end shut do not.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <decomp/libc.h>
#include <libgte.h>
#include <libgpu.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/draw.h>
#include <persona/btlp/effect.h>
#include <persona/btlp/input.h>
#include <persona/btlp/number.h>

/* Unity, and the two floors the eased pair stop at. */
#define EFFECT_SCALE_ONE 0x1000
#define EFFECT_SCALE_MIN 0x40
#define EFFECT_SHUT      5

/* What the fixed-step pair move by. */
#define EFFECT_STEP_OUT 0x300
#define EFFECT_STEP_IN  0xC8

/* The kind byte the fully open box takes, high nibble kept. */
#define EFFECT_KIND_OPEN 0xF0

/* The decimal formatter writes 0 to 9 and pads with this; the glyph for a
   digit is that many past the one for zero. */
#define EFFECT_DIGIT_ZERO  0xC0
#define EFFECT_DIGIT_BLANK 0xCC

/* Already the size it wants. */
int BtlEffectMotionHold(BtlEffect *e)
{
    return 1;
}

/* Half again each frame, width first and then height, so the box springs
   open. The cursor comes back only once both have arrived. */
int BtlEffectMotionGrow(BtlEffect *e)
{
    BtlCursorShow(0);
    e->scale_x = e->scale_x + e->scale_x / 2;
    if (e->scale_x >= EFFECT_SCALE_ONE) {
        e->scale_x = EFFECT_SCALE_ONE;
        e->scale_y = e->scale_y + e->scale_y / 2;
        if (e->scale_y >= EFFECT_SCALE_ONE) {
            BtlCursorShow(1);
            e->scale_x = EFFECT_SCALE_ONE;
            e->scale_y = EFFECT_SCALE_ONE;
            e->scale = EFFECT_SCALE_ONE;
            e->kind &= 0xF0;
        }
    }
    return 1;
}

/* Full width from the first frame and the height let out a step at a time. */
int BtlEffectMotionUnroll(BtlEffect *e)
{
    e->scale_x = EFFECT_SCALE_ONE;
    e->scale_y += EFFECT_STEP_OUT;
    if (e->scale_y >= EFFECT_SCALE_ONE) {
        e->scale_x = EFFECT_SCALE_ONE;
        e->scale_y = EFFECT_SCALE_ONE;
        e->scale = EFFECT_SCALE_ONE;
        e->kind = EFFECT_KIND_OPEN;
    }
    return 1;
}

/* The same the other way, and this one does end: the box is gone. */
int BtlEffectMotionRoll(BtlEffect *e)
{
    e->scale_x = EFFECT_SCALE_ONE;
    e->scale_y -= EFFECT_STEP_IN;
    if (e->scale_y < 0) {
        e->scale_x = 0;
        e->scale_y = 0;
        e->scale = 0;
        e->kind = 0;
        return 0;
    }
    return 1;
}

int BtlEffectMotionOpenNow(BtlEffect *e)
{
    e->scale_x = EFFECT_SCALE_ONE;
    e->scale_y = EFFECT_SCALE_ONE;
    e->scale = EFFECT_SCALE_ONE;
    e->kind &= 0xF0;
    BtlCursorShow(1);
    return 1;
}

int BtlEffectMotionShutNow(BtlEffect *e)
{
    e->scale_x = 0;
    e->scale_y = 0;
    e->scale = 0;
    e->kind = 0;
    BtlCursorShow(0);
    return 0;
}

/* A third off the height each frame down to a sliver, then a third off the
   width, so the box folds away the way it sprang open. */
int BtlEffectMotionShrink(BtlEffect *e)
{
    BtlCursorShow(0);
    e->scale_y = e->scale_y - e->scale_y / 3;
    if (e->scale_y <= EFFECT_SCALE_MIN) {
        e->scale_x = e->scale_x - e->scale_x / 3;
        e->scale_y = EFFECT_SCALE_MIN;
        if (e->scale_x < EFFECT_SHUT) {
            e->scale_x = 0;
            e->scale_y = 0;
            e->scale = EFFECT_SCALE_ONE;
            e->kind = 0;
            return 0;
        }
    }
    return 1;
}

/* Both scales in together at a fixed step. It keeps answering yes: the pass
   that owns the record is the one that notices the box has gone. */
int BtlEffectMotionCollapse(BtlEffect *e)
{
    e->scale_x -= EFFECT_STEP_IN;
    e->scale_y -= EFFECT_STEP_IN;
    if (e->scale_x < 0) {
        e->scale_x = 0;
        e->scale_y = 0;
        e->scale = EFFECT_SCALE_ONE;
        e->kind = 0;
    }
    return 1;
}

/* The glyph quad is built in the scratchpad and copied into the frame's
   buffer, so only one of each is ever needed: the four corners first, then
   the quad whose screen coordinates RotTransPers4 writes into it. */
typedef struct {
    /* 0x00 */ SVECTOR  corner[4];
    /* 0x20 */ POLY_FT4 quad;
} BtlGlyphPad;                         /* 0x48 bytes */

/* The transform's two out-parameters are never read, so the corner counter
   shares their storage - and its low half is what the odd-corner test reads. */
typedef struct {
    /* 0x0 */ u_short vx;
    /* 0x2 */ u_short vy;
    /* 0x4 */ u_short vz;
    /* 0x6 */ u_short pad;
} BtlGlyphVec;                         /* 8 bytes */

#define GLYPH_PAD ((BtlGlyphPad *)0x1F800000)

/* The font is thirty-one cells across, each eight pixels wide and twelve
   tall, and a line ends at 0xFF. */
#define GLYPH_COLS 31
#define GLYPH_W    8
/* The column shifted rather than multiplied: written as a multiply the
   remainder is widened to a word first and the image does not widen it. */
#define GLYPH_W_SHIFT 3
#define GLYPH_H    12
#define GLYPH_END  0xFF
#define GLYPH_CORNERS 4

/* A raw-texture POLY_FT4, and where the glyphs sit in VRAM. */
#define GLYPH_LEN  9
#define GLYPH_CODE 0x2D
#define GLYPH_TP   0
#define GLYPH_ABR  0
#define GLYPH_VX   0x3C0
#define GLYPH_VY   0x100

/* One line of text, a glyph at a time. Each cell goes out as a quad through
   RotTransPers4 rather than a sprite, so a line can sit in perspective with
   whatever it is drawn on; g_btl_glyph_x walks along the line and
   g_btl_glyph_y stays where the caller put it. The answer is always one. */
int BtlDrawGlyphs(const u_char *text, short clut)
{
    BtlGlyphPad *pad;
    BtlGlyphVec  scratch;
    int          cell;
    short        x;
    short        y;

    pad  = GLYPH_PAD;
    cell = 0;
    setlen(&pad->quad, GLYPH_LEN);
    setcode(&pad->quad, GLYPH_CODE);
    pad->quad.tpage = getTPage(GLYPH_TP, GLYPH_ABR, GLYPH_VX, GLYPH_VY);
    pad->quad.clut  = clut;
    while (*text != GLYPH_END) {
        y = g_btl_glyph_y;
        x = g_btl_glyph_x + cell * GLYPH_W;
        for (*(long *)&scratch = 0; *(long *)&scratch < GLYPH_CORNERS;
             (*(long *)&scratch)++) {
            pad->corner[*(long *)&scratch].vx = x + (scratch.vx & 1) * GLYPH_W;
            pad->corner[*(long *)&scratch].vy =
                y + (*(long *)&scratch >> 1) * GLYPH_H;
            pad->corner[*(long *)&scratch].vz = 0;
        }
        RotTransPers4(&pad->corner[0], &pad->corner[1], &pad->corner[2],
                      &pad->corner[3], (long *)&pad->quad.x0,
                      (long *)&pad->quad.x1, (long *)&pad->quad.x2,
                      (long *)&pad->quad.x3, (long *)&scratch,
                      (long *)&scratch);
        pad->quad.u0 = *text % GLYPH_COLS << GLYPH_W_SHIFT;
        pad->quad.v0 = *text / GLYPH_COLS * GLYPH_H;
        pad->quad.u1 = (*text % GLYPH_COLS << GLYPH_W_SHIFT) + GLYPH_W;
        pad->quad.v1 = *text / GLYPH_COLS * GLYPH_H;
        pad->quad.u2 = *text % GLYPH_COLS << GLYPH_W_SHIFT;
        pad->quad.v2 = *text / GLYPH_COLS * GLYPH_H + GLYPH_H;
        pad->quad.u3 = (*text % GLYPH_COLS << GLYPH_W_SHIFT) + GLYPH_W;
        pad->quad.v3 = *text / GLYPH_COLS * GLYPH_H + GLYPH_H;
        memcpy(g_btl_prim_next, &pad->quad, sizeof(POLY_FT4));
        addPrim(g_btl_effect_ot, g_btl_prim_next);
        text++;
        g_btl_prim_next += sizeof(POLY_FT4);
        cell++;
    }
    return 1;
}

/* One row of an effect, in whichever of the eight colours its kind picks. */
void BtlEffectDrawRow(const BtlEffectRow *row)
{
    BtlDrawGlyphs(row->text, g_btl_clut[EFFECT_CLUT + ((row->kind >> 4) & 7)]);
}

/* Eight hex digits and a terminator, leading zeros left blank - except the
   last, so a value of zero still writes one digit. */
void BtlFormatHexGlyphs(u_int value, u_char *out, int unused)
{
    u_char digit[16];
    int    i;
    int    seen;
    int    n;

    memcpy(digit, g_btl_hex_glyphs, sizeof(digit));
    for (i = 7, seen = 0; i >= 0; i--, out++) {
        n = (value >> (i * 4)) & 0xF;
        if (n == 0) {
            /* a zero is a digit like any other once the run has started */
            if (seen != 1) {
                goto blank;
            }
        } else {
            seen = 1;
        }
        *out = digit[n];
        continue;
    blank:
        if (i == 0) {
            *out = digit[0];
        } else {
            *out = 0;
        }
    }
    *out = 0xFF;
}

/* Not matched, both of these, and both down to the same last thing. Each is
   the right length with the right instructions in the right order; what is
   left is which register the allocator hands to which local - eight words in
   one and seventeen in the other, all of them a name rather than a value.

   Two things did most of the work getting them here, and are worth keeping
   in mind for the next one: the record is read through a second pointer of
   its own, and the digit buffer through a pointer taken again on every turn
   of the loop, which is what stops gcc walking it. */
/* A number read through the row's pointer and drawn where the row sits. The
   mask says how wide the value is, and the top bit of the kind byte picks hex
   over decimal - the decimal formatter writes plain digits, so they are
   carried up into the glyph codes afterwards, leaving the blank it pads with
   alone. */
#ifdef NON_MATCHING
void BtlEffectDrawNumber(const BtlEffectRow *row)
{
    const BtlEffectRow *r;
    u_char *p;
    u_char *base;
    u_char text[16];
    u_int  value;
    int    i;

    r = row;
    value = 0;
    switch (row->u.mask) {
    case 0xFF:
        value = *(const u_char *)row->text;
        break;
    case 0xFFFF:
        value = *(const u_short *)row->text;
        break;
    case -1:
        value = *(const u_long *)row->text;
        break;
    }
    if ((r->kind & 0x80) != 0) {
        BtlFormatHexGlyphs(value, text, 0);
    } else {
        do {
            BtlFormatDecimal(value, text, 0);
            i = 0;
        } while (0);
        base = text;
        p = base;
        if (p[0] != 0xFF) {
            do {
                if (p[i] != EFFECT_DIGIT_BLANK) {
                    p[i] += EFFECT_DIGIT_ZERO;
                }
                i++;
                p = base;
            } while (p[i] != 0xFF);
        }
        text[i] = 0xFF;
    }
    BtlDrawGlyphs(text, g_btl_clut[EFFECT_CLUT + ((row->kind >> 4) & 7)]);
}
#else
INCLUDE_ASM("btlp/nonmatchings/effectmotion", BtlEffectDrawNumber);
#endif

/* A run of lines out of the list the row points at, one under the next. It
   stops early on a line of -1, and on a drawer that says it could not fit
   what it was given. */
int BtlEffectDrawLines(const BtlEffectRow *row)
{
    const BtlEffectRow *rr;
    const BtlEffectRow *r;
    const u_char **line;
    const BtlEffectRowData *d;
    int n;

    r = row;
    n = 0;
    d = &r->u;
    rr = r;
    line = &((const u_char **)rr->text)[d->list.first];
    for (; n < rr->u.list.count; n++) {
        if (*line == (const u_char *)-1) {
            return 1;
        }
        if (BtlDrawGlyphs(*line,
                          g_btl_clut[EFFECT_CLUT + ((row->kind >> 4) & 7)])
            == 0) {
            return 0;
        }
        g_btl_glyph_y += 8;
        line++;
    }
    return 1;
}
