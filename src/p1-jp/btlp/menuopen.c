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
#include <types.h>

#define BTL_MENU_LIVE 2

/* Where the entries sit inside the box. */
#define MENU_ENTRY_X    6
#define MENU_ENTRY_Y    4
#define MENU_ENTRY_STEP 0x18

/* The strip of VRAM the entries' glyphs are staged in. */
#define MENU_STAGE_X 0x380
#define MENU_STAGE_Y 0x150
#define MENU_STAGE_W 0x40
#define MENU_STAGE_H 0x30

/* One entry: the text, and where it goes. */
typedef struct {
    /* 0x0 */ const u_char *text;
    /* 0x4 */ short         x;
    /* 0x6 */ short         y;
} BtlMenuCell;                      /* 8 bytes */

extern BtlMenuCell g_btl_menu_cells[];
extern int g_btl_menu_state;
extern int g_btl_menu_index;
extern int g_btl_menu_count;
extern int g_btl_menu_shift;
extern int g_btl_menu_slide;

extern void BtlUploadText(const BtlMenuCell *cell, const u_char *text);
extern void BtlQueueVramClear(short x, short y, short w, short h,
                              u_char r, u_char g, u_char b);
extern void BtlCursorInitPrims(void);
extern void BtlCursorShow(int on);

void BtlMenuOpen2(const u_char **text)
{
    BtlMenuCell *cell;
    BtlMenuCell *put;
    BtlMenuCell *p;
    const u_char *s;
    short        y;
    int          i;
    int          off;

    i = 0;
    cell = g_btl_menu_cells;
    y = MENU_ENTRY_Y;
    off = 0;
    g_btl_menu_state = BTL_MENU_LIVE;
    g_btl_menu_index = 0;
    g_btl_menu_count = 2;
    put = cell;
    /* The two coordinates go in through a byte offset rather than the cell
       pointer that is walking alongside them; that is what keeps the table's
       address folded into each store. */
    do {
        p = put;
        put++;
        cell->text = *text;
        text++;
        i++;
        *(short *)((char *)g_btl_menu_cells + off + 4) = MENU_ENTRY_X;
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

    i = 0;
    cell = g_btl_menu_cells;
    off = 0;
    g_btl_menu_state = BTL_MENU_LIVE;
    g_btl_menu_index = 0;
    g_btl_menu_count = 3;
    put = cell;
    do {
        p = put;
        put++;
        cell->text = *text;
        text++;
        y = i << 4;
        i++;
        *(short *)((char *)g_btl_menu_cells + off + 4) = MENU_ENTRY_X;
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
