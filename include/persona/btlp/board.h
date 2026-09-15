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

/* The Persona board the cast command picks a spell on, put up and taken down.
   personaboard.c. */
extern void BtlOpenPersonaBoard(void);
extern void BtlClosePersonaBoard(void);

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

/* That board's thirteen numbers as text rows - the eight derived values from
   melee_atk on, then the five stats - which the level-ups redraw in another
   palette as each one changes. */
extern BtlGfxText g_btl_edit_numbers[];

/* The results board a won fight is held on for a key. board1d.c. */
extern void    BtlOpenBoard1F(void);
extern void    BtlCloseBoard1F(void);

/* The names of the seven things the fighter is wearing, ten bytes each, as
   the editor's board shows them. equipnames.c. */
extern void    BtlEditEquipNames(BtlActor *a);

/* The board the editor's equipment page stands over, put up and taken
   down. board1d.c. */
extern void    BtlOpenBoard1D(void);
extern void    BtlCloseBoard1D(void);

/* A name as a board's cells take it, copied whole: ten glyph bytes, 0xFF
   ending the row. */
typedef struct {
    u_char b[10];
} BtlNameCells;

/* The label each arcana is shown by, by the 1-based PersonaData.arcana: eight
   bytes a row, of which a board takes the first six. */
extern u_char g_btl_arcana_labels[][8];

/* The board of the kinds of enemy in the fight, which R1 on the command picker
   holds up until a key comes: filled in and put up, and taken down.
   board23.c. */
extern void    BtlOpenBoard23(void);
extern void    BtlCloseBoard23(void);

/* Colours a fighter's two bars by how much is left - full, ordinary, or low at
   a quarter or under - on the clut byte of the cell or text row each points
   at. Either may be left out. gaugecolour.c. */
#define GAUGE_CLUT_AT 9
#define GAUGE_FULL    0x24
#define GAUGE_OK      0x20
#define GAUGE_LOW     0x22
#define GAUGE_LOW_AT  4
extern void BtlSetGaugeColour(const Char *c, u_char *hp, u_char *sp);

#endif
