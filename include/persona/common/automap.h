#ifndef PERSONA_COMMON_AUTOMAP_H
#define PERSONA_COMMON_AUTOMAP_H

/* Persona 1 (JP) - the automap's record of which tiles have been seen.
 *
 * Every room owns 72 bytes: a 24 by 24 bit grid, three bytes to a row, with
 * the most significant bit of a byte at the lower x. Rooms are numbered
 * globally, and g_map_base turns a map id into the index of its first room.
 * The grid is in the save and reached by hardcoded address.
 */
#include <decomp/types.h>

#define MAP_ROW_BYTES 3
#define MAP_BYTES     72

#define g_map_seen ((u_char *)0x801F2B38)

extern const u_char g_map_base[];

#endif
