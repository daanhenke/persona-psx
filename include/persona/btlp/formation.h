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
extern void BtlStandPreset(int preset);
extern void BtlPlacePreset(int preset);
extern int  BtlMarkMovedMembers(void);

/* Whether a member may stand at a cell: its four neighbours must be empty,
   the same rule the field keeps. */
extern int  BtlFormationCellFree(short col, short row);

/* Whether a stored layout has never been written to, and whether it places as
   many fighters as the live grid holds. formationpreset.c. */
extern int  BtlFormationPresetEmpty(int slot);
extern int  BtlFormationPresetFits(int slot);

/* Stands every member who went down back on the grid, one at a time, and
   the one frame of the cursor that does it. placefallen.c. */
extern void BtlPlaceFallen(void);
extern int  BtlPlaceFallenStep(void);

#endif
