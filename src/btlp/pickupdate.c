/* Persona 1 (JP) - one frame of the battle command list.  BTLP only.
 *   0x800A37A4 BtlPickUpdate
 *
 * The list is six rows on a page, and which of them can be chosen is
 * g_btl_pick_live. A direction moves the cursor through g_btl_pick_move -
 * three bytes per row, up, down and sideways - and keeps moving the same way
 * until it lands on a live row, so a greyed-out command is stepped over rather
 * than sat on. Every direction clicks, whether the row changes or not.
 *
 * The row's help line goes up unless the player has turned help off, and the
 * list is redrawn with the new row highlighted.
 *
 * The answer is the row itself on confirm, -1 on cancel, -2 on the third key,
 * and BTL_PICK_WAIT while nothing has been decided - which is also the answer
 * outright while the list is still animating in.
 */
#include <decomp/types.h>
#include <persona/btlp/object.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/input.h>
#include <persona/btlp/sound.h>

/* Rows to a page, and how the pages sit in the two tables. */
#define BTL_PICK_ROWS 6

/* Which way the cursor went, as g_btl_pick_move numbers its columns. Left and
   right share the third: the list is one column wide, so sideways means the
   row beside it whichever way the player pushed. */
#define PICK_UP   0
#define PICK_DOWN 1
#define PICK_SIDE 2

/* The pad bits BtlMenuKey answers with. Spelled out here rather than read from
   the control scheme's table, which is what the original does - the four
   directions are the part of that table neither scheme changes. */
#define PAD_UP    0x1000
#define PAD_DOWN  0x4000
#define PAD_LEFT  0x8000
#define PAD_RIGHT 0x2000
#define PAD_ANY_DIRECTION 0xF000

/* The click. */
#define PICK_CLICK_BANK 1
#define PICK_CLICK_SE   0

/* Where the help line sits. */
#define PICK_HELP_X 0x10
#define PICK_HELP_Y 0x94

/* Nothing decided this frame. */
#define BTL_PICK_WAIT (-0x100)

extern BtlObj      *g_btl_pick_objs[];
extern const u_char g_btl_pick_move[][3];
extern const u_char g_btl_pick_live[][BTL_PICK_ROWS];
extern const u_long g_btl_pick_help[][BTL_PICK_ROWS];
extern u_char       g_btl_pick_page;
extern u_char       g_btl_no_help;

extern void  BtlOpenMessage(int flags, short style, const u_char *script,
                            short x, short y);
extern void  BtlCloseMessage(int slot);
extern void  BtlPickHighlight(int chosen);

int BtlPickUpdate(short *row)
{
    u_short keys;
    u_short edge;
    int     dir;
    u_char  next;

    dir  = -1;
    keys = BtlMenuKey();

    if ((keys & PAD_ANY_DIRECTION) != 0) {
        BtlSePlay(PICK_CLICK_BANK, PICK_CLICK_SE);
    }
    if ((keys & PAD_UP) != 0) {
        dir = PICK_UP;
    }
    if ((keys & PAD_DOWN) != 0) {
        dir = PICK_DOWN;
    }
    if ((keys & PAD_LEFT) != 0) {
        dir = PICK_SIDE;
    }
    if ((keys & PAD_RIGHT) != 0) {
        dir = PICK_SIDE;
    }

    while (dir >= 0) {
        next = g_btl_pick_move[*row][dir];
        *row = next;
        if (g_btl_pick_live[g_btl_pick_page][next] != 0) {
            break;
        }
    }

    if (g_btl_no_help == 0) {
        BtlOpenMessage(0, 0,
                       (const u_char *)g_btl_pick_help[g_btl_pick_page][*row],
                       PICK_HELP_X, PICK_HELP_Y);
    } else {
        BtlCloseMessage(0);
    }
    BtlPickHighlight(*row);

    if (g_btl_pick_objs[0]->motion != 0) {
        return BTL_PICK_WAIT;
    }
    edge = g_btl_pad1_edge;
    if ((edge & g_btl_key_confirm) != 0) {
        return *row;
    }
    if ((edge & g_btl_key_cancel) != 0) {
        return -1;
    }
    if ((edge & g_btl_key_abort) != 0) {
        return -2;
    }
    return BTL_PICK_WAIT;
}
