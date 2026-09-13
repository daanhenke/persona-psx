/* Persona 1 (JP) - the windows the battle puts up whole.
 *
 * A board is built from a table of four records - two shadow pieces, the
 * window itself and a border - each allocated from a template, chained
 * through BtlObj.attached and given the same position. BtlBoardOpen returns
 * the top of that chain and starts it growing into place; BtlBoardShut takes
 * that record back and starts it shrinking away. Nothing is freed either way:
 * the records go idle at the end of their motion.
 *
 * The tables differ only in the index the window record takes, which is which
 * picture the board is drawn from - so a board is named after that index
 * wherever there is nothing better to call it by.
 */
#ifndef PERSONA_BTLP_BOARD_H
#define PERSONA_BTLP_BOARD_H

#include <decomp/types.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/object.h>

/* One record of a board: the template it comes from, the index that picks the
   picture, the kind, and the two bytes that land at +0xCD and +0xCE. The same
   eight bytes a party marker is described by. */
typedef struct {
    /* 0x0 */ const BtlObjDef *defs;
    /* 0x4 */ u_char index;
    /* 0x5 */ u_char kind;
    /* 0x6 */ u_char p7;
    /* 0x7 */ u_char p8;
} BtlBoardDef;                         /* 8 bytes */

/* Not const: a first record of index 9 is rewritten to the editor picture
   D_800CCA24 picks, in the table itself, as the board goes up. boardopen.c. */
extern BtlObj *BtlBoardOpen(BtlBoardDef *parts, const long *pos);
extern void    BtlBoardShut(BtlObj *board);

/* The stock board the Persona change is picked on, filled in from the acting
   member and put up, and taken down. stockboard.c. */
extern void BtlOpenStockBoard(void);
extern void BtlCloseStockBoard(void);

/* The boards a menu outside boards.c puts up: the debug page, the switch
   board of its flag editor, and the status view an analysis is shown on. */
extern void BtlOpenDebugBoard(void);
extern void BtlCloseDebugBoard(void);
extern void BtlOpenFlagBoard(void);
extern void BtlCloseFlagBoard(void);

/* The standing orders board and the tactics board. menuboards.c. */
extern void BtlOpenOrdersBoard(void);
extern void BtlCloseOrdersBoard(void);
extern void BtlCloseTacticsBoard(void);
extern void BtlOpenStatusBoard(void);
extern void BtlCloseStatusBoard(void);

/* The debug character editor's board: put up, taken down, and filled in from
   one fighter. editboard.c. */
extern BtlObj *BtlOpenEditBoard(void);
extern void    BtlShutEditBoard(void);
extern void    BtlFillEditBoard(BtlActor *a);

/* The names of the seven things the fighter is wearing, ten bytes each, as
   the editor's board shows them. equipnames.c. */
extern void    BtlEditEquipNames(BtlActor *a);

/* The board the editor's equipment page stands over, put up and taken
   down. board1d.c. */
extern void    BtlOpenBoard1D(void);
extern void    BtlCloseBoard1D(void);

#endif
