/* Persona 1 (JP) - putting the tiled background map back to nothing.  ADV only.
 *   ADV 0x800667B8
 *
 * Wipes the map's texture page in VRAM, empties the 64-word table beside the
 * map, points every cell at the blank corner of the page, queues the map's
 * palette row for upload and blanks the four rows of the map index - the
 * last written out a row at a time rather than through BgMapClearRow.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <persona/common/bg.h>
#include <persona/common/item.h>
#include <persona/common/menulist.h>
#include <persona/common/pad.h>
#include <persona/common/persona.h>
#include <persona/common/slot.h>
#include <persona/common/spell.h>
#include <persona/common/imageanim.h>
#include <persona/common/vram.h>

/* The texture page the cells are cut from, and the palette row under it. */
#define PAGE_X   0x3C0
#define PAGE_Y   0x100
#define PAGE_W   0x40
#define PAGE_H   0x100
#define CLUT_Y   0x1FF

/* Every cell the map can point at, plus the blank one ahead of them. */
#define BG_CELLS (BG_MAP_W * BG_MAP_H + 5)

#define CELL_CBA   0x7FFC
#define CELL_TPAGE 0x1F

/* Beside the cells in the work area, reached by hardcoded address. */
#define BG_SLOTS 64
#define g_bg_slots ((u_int *)(0x800EB14C + WORK_BIAS))

/* The palette row the map's cells draw with. */
extern u_long g_bg_clut[];

static inline void ClearRow(u_short row)
{
    int i;

    for (i = 0; i < BG_MAP_W; i++) {
        g_bg_index[row * BG_MAP_W + i] = 0;
    }
}

void BgReset(void)
{
    u_int  *slots;
    GsCELL *cell;
    u_char  n;
    int     q;
    /* Eight bytes of frame nothing reads; neither the inline's parameter nor
       any local's type accounts for them. */
    int     unused[2];

    slots = g_bg_slots;
    cell = g_bg_cells;
    VramClearRect(PAGE_X, PAGE_Y, PAGE_W, PAGE_H);
    for (n = 0; n < BG_SLOTS; n++) {
        slots[n] = 0;
    }
    for (n = 0; n < BG_CELLS; n++) {
        cell->u = 0;
        cell->v = 0;
        cell->cba = CELL_CBA;
        cell->flag = 0;
        cell->tpage = CELL_TPAGE;
        cell++;
    }

    q = g_image_queue_count;
    g_image_queue[q].data = g_bg_clut;
    g_image_queue[q].rect.x = PAGE_X;
    g_image_queue[q].rect.y = CLUT_Y;
    g_image_queue[q].rect.w = PAGE_W;
    g_image_queue[q].rect.h = 1;
    g_image_queue_count = q + 1;

    ClearRow(0);
    ClearRow(1);
    ClearRow(2);
    ClearRow(3);
}

/* ------------------------------------------------------------------------ */
/* The message interpreter.  ADV 0x80066970.
 *
 * Called once a frame while a message is open. It waits out any pause, the
 * glyph delay (skipped while a button is held) or a key prompt, then takes one
 * step of the script: a glyph into the next cell of the window, or a control
 * code. A control code is SCRIPT_CODE followed by its number:
 *
 *    1 end the message, or the string being inserted
 *    2 wait for a key          3 newline          4 clear the window
 *    5 pause, two bytes        6 set the palette nibble
 *    7, 15, 16  a member's surname, first name, or both
 *    8-12, 19  an inserted name from the tables below, `left` glyphs long
 *   13 continue from another script      14 open a choice
 *   17 the second money count, 18 how many of item 0x23 are held
 *
 * An inserted string is read through `sub` with MSG_SUB set; the message
 * carries on from `script` once it runs out. Returns 1 once the message has
 * ended.
 */

#define SCRIPT_CODE  0xFF

/* Where the glyph for the cell after `n` is drawn in the font page. */
#define GLYPH_X(n)   ((((n) + 1) & 0xF) * 4 | 0x3C0)
#define GLYPH_Y(n)   ((((n) + 1) & ~0xF) + 0x100)

#define WINDOW_CELLS 60
#define ROW_CELLS    15
#define FULL_AT      30

#define CHOICE_SLOT  0x2E
#define COUNT_ITEM   0x23

typedef struct {
    u_char *script;
    u_char  rows;
    u_char  cols;
    u_char  count;
    u_char  pad;
} MsgChoice;

typedef struct {
    u_char  unk0;
    u_char  unk1;
    u_char  pad[6];
} MsgMark;

/* Each member's first name and surname, ten glyphs each, by Char.key. */
#define MEMBER_NAMES 20
#define FIRST_NAME   0
#define SURNAME      10

extern u_char      g_msg_answer;
extern u_long      g_msg_clut[];
extern u_long     *g_msg_blink[];
extern u_char      g_msg_choice_cur_def[];
extern MsgMark     g_msg_marks[];
extern MsgChoice   g_msg_choices[];
extern u_char     *g_msg_scripts[];
extern u_char      g_member_names[];
extern u_char      g_insert8_names[][40];
extern u_char      g_kind_labels[];
extern const u_int g_pow10[];
extern u_char      InputCheckAcceptA(u_char repeat);

#define g_money2 (*(u_int *)0x801F2678)

void FontUploadGlyph(short x, short y, u_short glyph);

/* FormatDecimal's body, which this file carries as an inline of its own:
   the digits least significant first, and how many of them count. */
static inline short FormatDigits(u_int value, u_char *dst, u_short width)
{
    u_short i;
    u_short last;

    last = 0;
    for (i = 0; i < width; i++) {
        *dst = 0;
        dst++;
    }
    dst--;
    if (i != 0) {
        do {
            if (value >= g_pow10[i - 1]) {
                *dst = value / g_pow10[i - 1];
                value = value % g_pow10[i - 1];
            }
            i--;
            dst--;
        } while (i != 0);
    }
    for (i = 0; i < width; i++) {
        dst++;
        if (*dst != 0) {
            last = i;
        }
    }
    last++;
    return last;
}

/* 99.62%: case 1 keeps the script pointer in v1 where the original has s2,
   so its surname hand-off is not cross-jumped with the glyph tail's. */
#ifdef NON_MATCHING
int MsgStep(void)
{
    MsgState *st = g_msg;
    u_char   *s;
    u_short   c;
    RECT      clut = { 0x20, 0x1E2, 11, 1 };
    u_char    i;
    u_short   k;
    u_int     count;
    int       ans;
    u_int    *slots;

    slots = g_bg_slots;
    for (i = 0; i < BG_SLOTS; i++) {
        slots[i] = 0;
    }
    if (st->flags & MSG_SUB) {
        s = st->sub;
    } else {
        s = st->script;
    }
    if (st->flags & MSG_DONE) {
        return 1;
    }
    c = 0;
    if (st->flags & MSG_SURNAME) {
        s = st->script;
        s = &g_member_names[(*s - 1) * MEMBER_NAMES + SURNAME];
        st->flags = (st->flags ^ MSG_SURNAME) | MSG_SUB;
        st->left = 5;
        goto glyph;
    }
    if (st->flags & MSG_CHOICE) {
        if (!MenuStepCursor(&st->choice[0])) {
            MenuStepCursor(&st->choice[1]);
        }
        SlotSetPos(CHOICE_SLOT, 0x1E,
                   (st->choices + 1) * st->choice[1].cur * 16 + 0x28,
                   st->choice[0].cur * 16 + 0xB4);
        if (InputCheckAcceptA(1)) {
            st->flags ^= MSG_CHOICE;
            if (st->choice[0].hi != 0) {
                if (st->choice[1].hi != 0) {
                    ans = (st->choice[1].hi + 1) * st->choice[0].cur
                        + (u_char)st->choice[1].cur;
                } else {
                    ans = (u_char)st->choice[0].cur;
                }
            } else if (st->choice[1].hi == 0) {
                ans = (u_char)st->choice[0].cur;
            } else {
                ans = (u_char)st->choice[1].cur;
            }
            g_msg_answer = ans;
            SlotClear(CHOICE_SLOT);
            QueueImageUpload((u_short *)&clut, g_msg_clut);
            ImageAnimStop(0);
        }
        goto done;
    }
    if (st->wait != 0) {
        st->wait--;
        goto ret;
    }
    if (!(st->flags & MSG_KEY)) {
        if (st->delay != 0 && !g_pad_held[0]) {
            st->delay--;
            goto ret;
        }
    } else {
        if (!g_pad_pressed[0]) {
            goto done;
        }
        st->flags &= ~MSG_KEY;
        QueueImageUpload((u_short *)&clut, g_msg_clut);
        ImageAnimStop(0);
    }

next:
    if (st->flags & MSG_SCROLL) {
        g_bg_layers[4].scrolly += 4;
        if (g_bg_layers[4].scrolly & 0xF) {
            goto done;
        }
        ClearRow(((st->cursor + ROW_CELLS) / ROW_CELLS) & 3);
        st->flags &= ~MSG_SCROLL;
        goto done;
    }
    if (st->flags & MSG_COUNTED) {
    insert:
        c = *s;
        if (st->flags & MSG_BACKWARD) {
            s--;
        } else {
            s++;
        }
        if (c == SCRIPT_CODE || st->left == 0) {
            if (!(st->flags & MSG_SUB)) {
                st->flags |= MSG_DONE;
                goto done;
            }
            if (st->flags & MSG_BACKWARD) {
                st->flags ^= MSG_BACKWARD;
            }
            s = st->script + 1;
            st->flags = (st->flags & 0xFFFE) ^ MSG_COUNTED;
            goto done;
        }
        FontUploadGlyph(GLYPH_X(st->cursor), GLYPH_Y(st->cursor), c);
        BgMapSetCell(st->cursor, c);
        st->cursor++;
        st->delay = st->speed;
        if (!(st->flags & MSG_FULL)) {
            if (st->cursor >= FULL_AT) {
                st->flags |= MSG_FULL;
            }
        } else if (st->cursor % ROW_CELLS == 0) {
            st->flags |= MSG_SCROLL;
        }
        st->left--;
        goto done;
    }

    c = *s++;
    if (c != SCRIPT_CODE) {
        goto glyph;
    }
    c = *s++;
    switch (c) {
    case 1:
        if (st->flags & MSG_SUB) {
            if (st->flags & MSG_CHOOSING) {
                st->flags |= MSG_CHOICE;
                ImageAnimStart(0, g_msg_blink, 0x20, 0x1E2, 11, 1);
                SlotInitTagged(g_msg_choice_cur_def, CHOICE_SLOT, 0x1E, 0x28,
                               0xB4);
                SlotSetFlicker(CHOICE_SLOT, 1);
            }
            if (st->flags & MSG_FIRST) {
                st->flags = (st->flags ^ MSG_FIRST) | MSG_SURNAME;
                goto done;
            }
            s = st->script;
            st->flags &= ~(MSG_SUB | MSG_CHOOSING | MSG_SURNAME);
            s++;
        } else {
            st->flags |= MSG_DONE;
        }
        break;
    case 2:
        st->flags |= MSG_KEY;
        ImageAnimStart(0, g_msg_blink, 0x20, 0x1E2, 11, 1);
        break;
    case 3:
        /* The start of the next row, wrapping after the fourth. */
        st->cursor = (st->cursor + ROW_CELLS) % WINDOW_CELLS
                   - (st->cursor + ROW_CELLS) % ROW_CELLS;
        if (st->flags & MSG_FULL) {
            st->flags |= MSG_SCROLL;
        }
        if (!(st->flags & MSG_FULL) && st->cursor >= FULL_AT) {
            st->flags |= MSG_FULL;
        }
        st->flags |= MSG_NEWLINE;
        break;
    case 4:
        g_bg_layers[4].scrolly = 0;
        st->cursor = 0;
        ClearRow(0);
        ClearRow(1);
        ClearRow(2);
        ClearRow(3);
        st->flags &= ~MSG_FULL;
        break;
    case 5:
        st->wait = *s++;
        st->wait += *s++ << 8;
        break;
    case 6:
        st->flags = (st->flags & 0xFC0F) + (*s++ << 4);
        break;
    case 8:
        st->script = s;
        s = g_insert8_names[*s];
        st->left = 8;
        st->flags |= MSG_SUB | MSG_COUNTED;
        goto insert;
    case 9:
        c = *s++;
        st->script = s;
        s = g_item_defs[c | *s << 8].name;
        st->left = 10;
        st->flags |= MSG_SUB | MSG_COUNTED;
        goto insert;
    case 10:
        st->script = s;
        s = g_spell_data[*s].name;
        st->left = 10;
        st->flags |= MSG_SUB | MSG_COUNTED;
        goto insert;
    case 11:
        st->script = s;
        s = g_persona_data[*s].name;
        st->left = 10;
        st->flags |= MSG_SUB | MSG_COUNTED;
        goto insert;
    case 12:
        st->script = s;
        s = g_persona_defs[*s].unk08;
        st->left = 10;
        st->flags |= MSG_SUB | MSG_COUNTED;
        goto insert;
    case 7:
        st->script = s;
        s = &g_member_names[(*s - 1) * MEMBER_NAMES + SURNAME];
        st->left = 5;
        st->flags |= MSG_SUB;
        goto next;
    case 15:
        st->script = s;
        s = &g_member_names[(*s - 1) * MEMBER_NAMES + FIRST_NAME];
        st->left = 5;
        st->flags |= MSG_SUB;
        goto next;
    case 16:
        st->script = s;
        s = &g_member_names[(*s - 1) * MEMBER_NAMES + FIRST_NAME];
        st->left = 5;
        st->flags |= MSG_SUB | MSG_FIRST;
        goto next;
    case 14:
        c = *s;
        st->script = s;
        st->left = 0xFF;
        st->flags |= MSG_SUB | MSG_CHOOSING;
        st->choices = g_msg_choices[c].count;
        s = g_msg_choices[c].script;
        MenuListInit(&st->choice[0], 0, 0, g_msg_choices[c].rows - 1, 0x16);
        MenuListInit(&st->choice[1], 0, 0, g_msg_choices[c].cols - 1, 0x1A);
        for (i = 0; i < st->choices; i++) {
            g_msg_marks[i].unk0 = 0;
            g_msg_marks[i].unk1 = 0xB4;
        }
        for (i = st->choices; i < 15; i++) {
            g_msg_marks[i].unk0 = 0xFF;
            g_msg_marks[i].unk1 = 0xFF;
        }
        goto next;
    case 13:
        c = *s;
        st->script = s;
        s = g_msg_scripts[c];
        st->flags |= MSG_SUB;
        st->left = 0xFF;
        break;
    case 17:
        s--;
        st->script = s;
        st->left = FormatDigits(g_money2, st->digits, 8);
        for (i = 0; i < st->left; i++) {
            st->digits[i] += 0xC0;
        }
        s = &st->digits[st->left - 1];
        st->flags |= MSG_BACKWARD | MSG_COUNTED | MSG_SUB;
        break;
    case 18:
        s--;
        st->script = s;
        count = 0;
        k = ItemsFind(COUNT_ITEM);
        if ((short)k != -1) {
            count = g_items[(short)k] >> 9;
        }
        st->left = FormatDigits(count, st->digits, 2);
        for (i = 0; i < st->left; i++) {
            st->digits[i] += 0xC0;
        }
        s = &st->digits[st->left - 1];
        st->flags |= MSG_BACKWARD | MSG_COUNTED | MSG_SUB;
        break;
    case 19:
        st->script = s;
        s = &g_kind_labels[(g_persona_defs[*s].kind - 1) * 10];
        st->left = 10;
        st->flags |= MSG_SUB | MSG_COUNTED;
        goto insert;
    }
    goto done;

glyph:
    st->cursor %= WINDOW_CELLS;
    if (!(st->flags & MSG_FULL)) {
        if (st->cursor == FULL_AT) {
            st->flags |= MSG_FULL;
        }
    } else if (st->cursor % ROW_CELLS == 0) {
        if (!(st->flags & MSG_NEWLINE)) {
            st->flags |= MSG_SCROLL;
        } else {
            st->flags ^= MSG_NEWLINE;
        }
    }
    if (c < 0x80) {
        FontUploadGlyph(GLYPH_X(st->cursor), GLYPH_Y(st->cursor), c);
    } else {
        /* A two-byte glyph: the low three bits pick a bank of 256. */
        c = *s++ | (c & 7) << 8;
        FontUploadGlyph(GLYPH_X(st->cursor), GLYPH_Y(st->cursor), c);
    }
    BgMapSetCell(st->cursor, c);
    st->cursor++;
    st->delay = st->speed;
    if (!(st->flags & MSG_SUB)) {
        st->script = s;
        goto ret;
    }
    st->left--;
    if (st->left != 0) {
        goto done;
    }
    if (st->flags & MSG_FIRST) {
        st->flags = (st->flags ^ MSG_FIRST) | MSG_SURNAME;
        goto done;
    }
    s = st->script;
    st->flags ^= MSG_SUB;
    s++;

done:
    if (st->flags & MSG_SUB) {
        st->sub = s;
    } else {
        st->script = s;
    }
ret:
    return 0;
}
#else
INCLUDE_ASM("adv/nonmatchings/gfx/bgreset", MsgStep);
#endif
