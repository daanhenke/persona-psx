#ifndef PERSONA_BTLP_FORMATION_H
#define PERSONA_BTLP_FORMATION_H

#include <decomp/types.h>
#include <persona/common/formation.h>

/* Persona 1 (JP) - the battle's own copy of the formation grid.
 *
 * Same shape as the field's grid in common/formation.h and the same cell
 * values, but a second copy: the overlay loading over the field code would
 * otherwise take the original away, so the battle keeps one in the resident
 * work area and hands it back when the fight ends.
 */

/* One whole grid. The saved copy is moved as one object, seven words at a
   time, rather than a cell at a time. */
typedef struct {
    u_char cell[GRID_CELLS];
} BtlFormation;

extern u_char g_btl_formation[];

/* The eight layouts the player can store, immediately before the live grid.
   Every reader in a loop reaches a layout as (g_btl_formation_preset +
   n)->cell[k]; indexing the table folds the row into the symbol's address and
   builds it somewhere else (formationpreset.c, placecursor.c). */
extern BtlFormation g_btl_formation_preset[];

extern BtlFormation g_btl_formation_saved;

/* Where a scripted encounter stands each character: one block per encounter,
   and within it a column and a row for every Char key.

   There is one table of ten blocks and two bases into it. Encounters under
   0x11 index it from its own start; 0x11 and above index it from a base nine
   blocks below, so those two land on blocks 8 and 9. The original materialises
   that biased address rather than subtracting, which is why it reads as a
   second array - and the address it materialises is not an object at all: it
   falls eight words into g_btl_enemy_motion, on the tail of that table. Both
   bases are kept as symbols because the image sets each up with a lui and an
   addiu of its own, which writing the bias as arithmetic on the first does
   not reproduce - gcc shares the lui and subtracts. */
#define BTL_PLACE_CHAR      2
#define BTL_PLACE_ENCOUNTER 0x14

extern const u_char g_btl_place_lo[];
extern const u_char g_btl_place_hi[];

/* What a cell of the grid is worth on the field. Thirty pixels to a column
   from -0x3C, twenty to a row from +0x3C for the party; the enemies are laid
   out the same way from their own origin, mirrored across the middle. Row 0 is
   the front row on both sides - the one nearest the other side.

   Positions are 16.16. An object's column is stored doubled, because the grid
   is kept to half-column resolution. */
#define PLACE_COL_W   0x1E
#define PLACE_COL_ORG (-0x3C)
#define PLACE_ROW_H   0x14
#define PLACE_ROW_ORG 0x3C
#define PLACE_FIXED   0x10000

extern void BtlPlaceMember(int slot, short col, short row);
extern void BtlPlaceFormation(void);
extern void BtlFormationCloseUp(void);

/* The placement menu's previews: one stored layout stood on the fighters
   themselves or shown on the menu's cursors, and the members who have moved
   since the menu opened on g_btl_formation_before. Layout PRESET_LIVE is the
   live grid, which sits where a ninth stored layout would. placecursor.c. */
#define PRESET_LIVE 8

extern u_char g_btl_formation_before[GRID_CELLS];

/* Whether the formation has been changed since g_btl_formation_before was
   taken. BtlPlaceMenu raises it as it hands back a formation a member was
   moved in, and the command entry offers to put the old one back on a cancel
   from the first member while it is raised. BtlStageCommand and that undo
   clear it. */
extern u_char g_btl_formation_moved;
extern void BtlStandPreset(int preset);
extern void BtlPlacePreset(int preset);
extern int  BtlMarkMovedMembers(void);

/* Whether a member may stand at a cell: its four neighbours must be empty,
   the same rule the field keeps. */
extern int  BtlFormationCellFree(short col, short row);

/* The same rule, and the cell itself empty, on a copy of the grid with every
   member down where it stood put back on its cell. cellfree.c. */
extern BtlFormation g_btl_formation_fallen;
extern int  BtlFormationCellFreeOfFallen(short col, short row);

/* Whether a stored layout has never been written to, and whether it places as
   many fighters as the live grid holds. formationpreset.c. */
extern int  BtlFormationPresetEmpty(int slot);
extern int  BtlFormationPresetFits(int slot);

/* The placement cursor shared by placing the fallen and moving a member: the
   grid's anchor and where it is drawn for a cell, in pixels and gaps, the
   cell the cursor is on and what it holds, the member being placed, and the
   pick grid's voice slot with the sequences it plays. */
#define PLACE_X      0xE7
#define PLACE_Y      0x77
#define PLACE_XPITCH 16
#define PLACE_YPITCH 8

#define PLACE_SE_SLOT 2
#define PLACE_SE_OPEN 2
#define PLACE_SE_SHUT 3
#define PLACE_SE_PUT  4

struct BtlObj;
struct BtlActor;

extern struct BtlObj *g_btl_grid_anchor;
extern short          g_btl_place_member;
extern short          g_btl_place_cell;
extern short          g_btl_place_col;
extern short          g_btl_place_row;

/* One frame of moving a standing member in the placement menu. placestep.c. */
extern int BtlPlaceMoveStep(struct BtlActor *a);

/* Stands every member who went down back on the grid, one at a time, and
   the one frame of the cursor that does it. placefallen.c. */
extern void BtlPlaceFallen(void);
extern int  BtlPlaceFallenStep(void);

/* The placement menu, and the frame of its grid cursor it runs - choosing a
   member to pick up, or carrying one. The cursor answers 0 as a member is
   picked up or put down, -1 on a cancel, -2 once the player is done moving
   and BTL_PICK_WAIT otherwise. placemenu.c, and the cursor in asm. */
extern int BtlPlaceMenu(void);
extern int BtlPlaceGridUpdate(int carrying);

/* Raised by the third key anywhere in the placement menu: every step it
   passes through on the way back takes it as its own abort. */
extern int g_btl_place_abort;

/* The placement menu's lines: nobody left who can move, a stored layout
   refused while a member is down or has already moved, whether the new
   formation is kept, and whether a stored layout is written over. */
extern const u_char g_btl_msg_nobody_to_move[];
extern const u_char g_btl_msg_place_blocked[];
extern const u_char g_btl_msg_keep_formation[];
extern const u_char g_btl_msg_overwrite_layout[];

/* The board the stored layouts are chosen on. formationboard.c. */
extern void BtlOpenFormationBoard(void);
extern void BtlCloseFormationBoard(void);

#endif
