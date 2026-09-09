#ifndef _PERSONA_COMMON_FORMATION_H
#define _PERSONA_COMMON_FORMATION_H

/* Persona 1 - the battle formation grid.
 *
 * The party stands on a 5x5 grid. g_formation is the grid itself, one byte per
 * cell holding a party index or 0xFF for empty; g_formation_cell is the inverse
 * map, one cell index per party member. FormationCellFree is where the
 * placement rule lives: a cell is only usable when it and all four of its
 * orthogonal neighbours are empty, which is why a five-member party cannot
 * fill the middle.
 *
 * Eight saved layouts sit in the save-game work area at g_formation_preset,
 * 25 bytes each, in the same cell-to-member form as the grid.
 *
 * The grid is built into DNG, ADV and S2D. S2D's work area sits 0x20000
 * higher, which is what WORK_BIAS says. The code is two units per overlay:
 * the grid itself in formation.c, and the member sprites in
 * formationmembers.c.
 */
#include <decomp/types.h>
#include <persona/common/slot.h>

#define g_formation         ((u_char *)(0x800EB34C + WORK_BIAS))
#define g_formation_cell    ((u_char *)(0x800EB380 + WORK_BIAS))
#define g_formation_scratch ((u_char *)(0x800EB365 + WORK_BIAS))
#define g_formation_preset  ((u_char *)0x801F2584)

#define GRID_W     5
#define GRID_H     5
#define GRID_CELLS 25
#define PARTY_MAX  5
#define CELL_EMPTY 0xFF

/* The five grid markers live in slots 27..31, laid out sixteen pixels apart
   across and eight down from the grid's top left. The slot record is reached
   from the first marker's address rather than through g_slots, which is what
   the rematerialised base in the loop says the original did. */
#define g_marker_slot ((Slot *)(0x800DC838 + WORK_BIAS))
#define MARKER_SLOT   0x1B
#define MARKER_Z      6
#define MARKER_X0     0xD8
#define MARKER_Y0     0x10
#define MARKER_XPITCH 16
#define MARKER_YPITCH 8

/* The party's own markers, in slots 2..6, are drawn on the grid isometrically
   rather than on the flat layout the position markers use: eleven pixels of x
   per column against nine per row, four of y per column against three per row.
   The sort depth counts down from 0x18 so a nearer row draws in front. */
#define g_member_slot ((Slot *)(0x800DC194 + WORK_BIAS))
#define MEMBER_SLOT   2
#define MEMBER_X0     0x84
#define MEMBER_Y0     0x35
#define MEMBER_Z0     0x18
#define MEMBER_COL_X  11
#define MEMBER_ROW_X  9
#define MEMBER_COL_Y  4
#define MEMBER_ROW_Y  3

/* One texture holds every face; the slot's u offset picks one. */
#define MEMBER_PORTRAIT_W 16

/* Reached by hardcoded address here rather than through the linker symbol. */
#define g_party_at ((u_char *)0x801F256C)

/* Highest occupied party index, so the party holds g_party_last + 1 members. */
extern u_char g_party_last;

extern Slot *g_slot_cur;
/* One sprite definition per party member. */
extern void *g_formation_marker_def[];
extern void  g_formation_member_def;

extern u_char FormationCellOf(u_char member);
extern void   FormationSyncCells(void);
extern u_char FormationOtherAt(u_char member, u_char cell);
extern u_char FormationPresetCellOf(u_char member, u_char preset);
extern void   FormationLoadPreset(u_char preset);
extern u_char FormationCellFree(u_char cell);
extern int    FormationPresetFits(u_char preset);
extern u_char FormationFirstFree(void);
extern void   FormationPlaceMarkers(void);
extern void   FormationRepair(void);

extern void   FormationCompact(void);
extern void   FormationSetMemberSprite(u_char member, u_char portrait);
extern void   FormationDrawMembers(void);
extern void   FormationClearMarkers(void);

#endif
