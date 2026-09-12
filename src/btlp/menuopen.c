/* Persona 1 (JP) - putting entries in the choice box.  BTLP only.
 *   0x8007D460 BtlMenuOpen2  0x8007D688 BtlMenuOpenChoices
 *
 * Two entries, laid out down the box 0x18 apart from y = 4 and all at x = 6.
 * Each is one of the same eight-byte cells the contact box uses - the text and
 * where it goes - and the text is uploaded as the cell is filled in, so the
 * caller only hands over a list of pointers.
 *
 * Both displacements go back to zero, which is what brings a box that had been
 * slid out of the way back square before it is shown.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <persona/btlp/menu.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/input.h>
#include <persona/btlp/talk.h>
#include <persona/btlp/window.h>

/* Where the entries sit inside the box. */
#define MENU_ENTRY_X    6
#define MENU_ENTRY_Y    4
#define MENU_ENTRY_STEP 0x18

/* The strip of VRAM the entries' glyphs are staged in. */
#define MENU_STAGE_X 0x380
#define MENU_STAGE_Y 0x150
#define MENU_STAGE_W 0x40
#define MENU_STAGE_H 0x30

extern int g_btl_menu_shift;

extern void BtlUploadText(const BtlMenuCell *cell, const u_char *text);
extern void BtlQueueVramClear(short x, short y, short w, short h,
                              u_char r, u_char g, u_char b);

void BtlMenuOpen2(const u_char **text)
{
    BtlMenuCell *cell;
    BtlMenuCell *put;
    BtlMenuCell *p;
    const u_char *s;
    short        y;
    int          i;
    int          off;
    int          x;
    const u_char **next;

    /* Keep the fixed column and text walker explicit so setup is emitted
       in the original order. */
    i = 0;
    x = MENU_ENTRY_X;
    cell = g_btl_menu_cells;
    put = cell;
    y = MENU_ENTRY_Y;
    next = text;
    off = 0;
    g_btl_menu_state = BTL_MENU_LIVE;
    g_btl_menu_index = 0;
    g_btl_menu_count = 2;
    /* The two coordinates go in through a byte offset rather than the cell
       pointer that is walking alongside them; that is what keeps the table's
       address folded into each store. */
    do {
        p = put;
        put++;
        cell->text = *next;
        next++;
        i++;
        *(short *)((char *)g_btl_menu_cells + off + 4) = x;
        *(short *)((char *)g_btl_menu_cells + off + 6) = y;
        s = cell->text;
        cell++;
        y += MENU_ENTRY_STEP;
        off += 8;
        BtlUploadText(p, s);
    } while (i < 2);
    g_btl_menu_shift = 0;
    g_btl_menu_slide = 0;
    BtlQueueVramClear(MENU_STAGE_X, MENU_STAGE_Y, MENU_STAGE_W, MENU_STAGE_H,
                      0, 0, 0);
    BtlCursorInitPrims();
    BtlCursorShow(1);
}

/* Three entries instead of two, and a tighter layout: 0x10 apart from the top
   rather than 0x18 apart from y = 4. The line is worked out from the index
   here rather than carried along. */
void BtlMenuOpen3(const u_char **text)
{
    BtlMenuCell *cell;
    BtlMenuCell *put;
    BtlMenuCell *p;
    const u_char *s;
    int          i;
    int          y;
    int          off;
    int          x;
    const u_char **next;

    i = 0;
    x = MENU_ENTRY_X;
    cell = g_btl_menu_cells;
    put = cell;
    next = text;
    off = 0;
    g_btl_menu_state = BTL_MENU_LIVE;
    g_btl_menu_index = 0;
    g_btl_menu_count = 3;
    do {
        p = put;
        put++;
        cell->text = *next;
        next++;
        y = i << 4;
        i++;
        *(short *)((char *)g_btl_menu_cells + off + 4) = x;
        *(short *)((char *)g_btl_menu_cells + off + 6) = y;
        s = cell->text;
        cell++;
        off += 8;
        BtlUploadText(p, s);
    } while (i < 3);
    g_btl_menu_shift = 0;
    g_btl_menu_slide = 0;
    BtlQueueVramClear(MENU_STAGE_X, MENU_STAGE_Y, MENU_STAGE_W, MENU_STAGE_H,
                      0, 0, 0);
    BtlCursorInitPrims();
    BtlCursorShow(1);
}

/* The box the negotiation puts up, whose entries come out of the loaded pack
 * rather than from the caller. The row it is handed is six halfwords: how many
 * entries there are, and then a directory slot for each. Three is the most the
 * box holds, and a third entry whose slot is 0x1F4 or beyond is not one - the
 * pack uses those slots for something else - so the count comes down to two.
 *
 * Ten frames are run before anything is built, which is what lets the demon
 * finish whatever it was saying.
 *
 * Each entry gets a window of its own rather than a cell of typed-out text:
 * the script is walked, so the entry can carry the pack's own codes. A slot
 * that resolves past the end of the scratch area falls back to the table's
 * first entry rather than walking off it.
 */

/* The most entries the box holds, and the count a long third slot brings it
   down to. */
#define CHOICE_MAX      3
#define CHOICE_NO_THIRD 0x1F4
#define CHOICE_TWO      2

/* Frames run before the box is built. */
#define CHOICE_SETTLE 10

/* The tighter layout the three-entry box uses, and the last address the
   scratch area reaches. */
#define CHOICE_ENTRY_TIGHT 0x10
#define BTL_SCRATCH_LAST   0x801CFFFF

/* 95.69%: the pointer to the row is stored after the three slots rather than
   before them, and the loop's two byte offsets come out in each other's
   registers. Neither the walking-pointer form this file uses for the plain
   openers nor a byte offset for the row moved either one. */
#ifdef NON_MATCHING
void BtlMenuOpenChoices(u_short *row)
{
    BtlWindow    *w;
    const u_char *script;
    int           dir;
    int           i;

    /* The count is written back before it is read again, which is why the
       test below loads it a second time. */
    row[0] = row[0];
    if (row[0] > CHOICE_MAX) {
        row[0] = CHOICE_MAX;
    }
    g_btl_choice_at = row;
    g_btl_choice_lines[0] = row[1];
    g_btl_choice_lines[1] = row[2];
    g_btl_choice_lines[2] = row[3];

    BtlRunFrames(CHOICE_SETTLE);

    g_btl_menu_state = BTL_MENU_LIVE;
    g_btl_menu_index = 0;
    if (g_btl_choice_lines[2] >= CHOICE_NO_THIRD) {
        row[0] = CHOICE_TWO;
    }
    g_btl_menu_count = row[0];

    w = g_btl_choice_windows;
    for (i = 0; i < row[0]; i++) {
        if (i >= CHOICE_MAX) {
            break;
        }
        w->state  = WIN_SCRIPT;
        w->placed = 0;
        w->staged = 0;
        g_btl_menu_cells[i].x = MENU_ENTRY_X;
        if (row[0] == CHOICE_TWO) {
            g_btl_menu_cells[i].y = MENU_ENTRY_Y + i * MENU_ENTRY_STEP;
        } else {
            g_btl_menu_cells[i].y = i * CHOICE_ENTRY_TIGHT;
        }
        w->vram_x = g_btl_menu_cells[i].x * 2 + MENU_STAGE_X;
        w->vram_y = g_btl_menu_cells[i].y + MENU_STAGE_Y;

        dir    = g_btl_choice_text;
        script = BTL_SCRATCH + dir
                 + *(u_long *)(BTL_SCRATCH + dir + row[1 + i] * 4);
        if ((u_int)script > BTL_SCRATCH_LAST) {
            script = BTL_SCRATCH + dir + *(u_long *)(BTL_SCRATCH + dir);
        }
        w->script = script;

        BtlWindowStep(w, 0);
        w++;
    }

    g_btl_menu_shift = 0;
    g_btl_menu_slide = 0;
    BtlQueueVramClear(MENU_STAGE_X, MENU_STAGE_Y, MENU_STAGE_W, MENU_STAGE_H,
                      0, 0, 0);
    BtlCursorInitPrims();
    BtlCursorShow(1);
}
#else
INCLUDE_ASM("btlp/nonmatchings/menuopen", BtlMenuOpenChoices);
#endif
