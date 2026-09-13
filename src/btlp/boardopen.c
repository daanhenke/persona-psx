/* Persona 1 (JP) - putting a board up and taking it down.  BTLP only.
 *   0x800A7DE0 BtlBoardOpen  0x800A7F68 BtlBoardShut
 *
 * A board's table is a run of records ending in the window. BtlBoardOpen
 * allocates every record in front of the window, each chained behind the one
 * before through `attached` and marked as a trailing layer, and then the
 * window itself on top of them - which is the record it answers, set growing
 * into place on motion 3 from a sixty-fourth of its size.
 *
 * Three records go in front as a rule. The editor board's table has four; a
 * table whose first record has index nought has one fewer, and that record is
 * skipped. A first record with index 9 is really the editor's picture, and
 * which of the two pictures it takes is D_800CCA24's to say - the table is
 * written as the board goes up.
 *
 * BtlBoardShut starts the record BtlBoardOpen answered shrinking away on
 * motion 4, back down to the same sixty-fourth.
 */
#include <decomp/types.h>
#include <persona/btlp/board.h>
#include <persona/btlp/object.h>

extern u_char            D_800CCA24;
extern const BtlBoardDef g_btl_board1F_defs[];

/* Records in front of the window as a rule, and the two pictures a first
   record of index 9 stands for. */
#define BOARD_LAYERS    3
#define BOARD_PICTURE_A 9
#define BOARD_PICTURE_B 10

/* What the layers and the window are marked with, the group they come from,
   and how the window grows and shrinks. */
#define BOARD_LAYER_ATTR  0x400
#define BOARD_WINDOW_ATTR 0x1000
#define BOARD_GROUP       1
#define BOARD_SMALL       0x40
#define BOARD_DEPTH       0x1000
#define BOARD_GROW        3
#define BOARD_SHRINK      4
#define BOARD_SHUT_PHASE  1

BtlObj *BtlBoardOpen(BtlBoardDef *parts, const long *pos)
{
    BtlObj *o;
    BtlObj *after;
    int     layers;
    int     i;

    after = NULL;
    layers = BOARD_LAYERS;
    if (parts->index == BOARD_PICTURE_A) {
        parts->index = D_800CCA24 != 0 ? BOARD_PICTURE_A : BOARD_PICTURE_B;
    }
    if (parts == g_btl_board1F_defs) {
        layers++;
    }
    if (parts->index == 0) {
        parts++;
        layers--;
    }
    for (i = 0; i < layers; i++) {
        o = BtlObjAlloc(parts->defs, BOARD_GROUP, after, parts->kind,
                        parts->index, pos, parts->p7, parts->p8);
        o->attached = after;
        after = o;
        after->attr |= BOARD_LAYER_ATTR;
        parts++;
    }
    o = BtlObjAlloc(parts->defs, BOARD_GROUP, after, parts->kind, parts->index,
                    pos, parts->p7, parts->p8);
    o->attached = after;
    o->attr |= BOARD_WINDOW_ATTR;
    BtlObjSetScale(o, BOARD_SMALL, BOARD_SMALL, BOARD_DEPTH);
    BtlObjSetMotion(o, BOARD_GROW);
    return o;
}

void BtlBoardShut(BtlObj *board)
{
    BtlObjSetTimer(board, 0);
    BtlObjSetPhase(board, BOARD_SHUT_PHASE);
    BtlObjSetScaleTo(board, BOARD_SMALL);
    BtlObjSetMotion(board, BOARD_SHRINK);
}
