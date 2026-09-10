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

extern u_char g_btl_formation[];

/* The eight layouts the player can store, immediately before the live grid. */
extern u_char g_btl_formation_preset[];

/* The grid copied whole: the original moves it as one object, seven words at
   a time, rather than a cell at a time. */
typedef struct {
    u_char cell[GRID_CELLS];
} BtlFormation;

extern BtlFormation g_btl_formation_saved;

/* Where a scripted encounter stands each character: one block per encounter,
   and within it a column and a row for every Char key. The two tables split
   the encounter range between them. */
#define BTL_PLACE_CHAR      2
#define BTL_PLACE_ENCOUNTER 0x14

extern const u_char g_btl_place_lo[];
extern const u_char g_btl_place_hi[];

extern void BtlPlaceFormation(void);
extern void BtlFormationCloseUp(void);

#endif
