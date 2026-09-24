#ifndef PERSONA_COMMON_ITEMNAME_H
#define PERSONA_COMMON_ITEMNAME_H

/* Persona 1 (JP) - an item's name into a menu row (ADV 0x8008FB18).
 *
 * Writes the item's ten-cell name from glyph bank `base`; for item 0 it
 * writes the dimmed "nothing" one cell in when `nothing` is set, and leaves
 * the row alone otherwise.
 *
 * Typed as the routine takes its arguments. itemcell.c and itemrow.c still
 * declare it with int parameters: their calls were built against that
 * declaration, so they keep it rather than include this. */
#include <decomp/types.h>

void DrawItemName(short id, short *dst, u_short base, short nothing);

#endif
