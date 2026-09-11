/* Persona 1 (JP) - putting entries in the choice box.  BTLP only.
 *   0x8007D460 BtlMenuOpen2
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

/* Where the entries sit inside the box. */
#define MENU_ENTRY_X    6
#define MENU_ENTRY_Y    4
#define MENU_ENTRY_STEP 0x18

/* The strip of VRAM the entries' glyphs are staged in. */
#define MENU_STAGE_X 0x380
#define MENU_STAGE_Y 0x150
#define MENU_STAGE_W 0x40
#define MENU_STAGE_H 0x30

extern BtlMenuCell g_btl_menu_cells[];
extern int g_btl_menu_state;
extern int g_btl_menu_index;
extern int g_btl_menu_count;
extern int g_btl_menu_shift;
extern int g_btl_menu_slide;

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
