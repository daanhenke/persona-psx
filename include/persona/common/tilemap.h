#ifndef PERSONA_COMMON_TILEMAP_H
#define PERSONA_COMMON_TILEMAP_H

/* Persona 1 (JP) - the background character-map layers and their writers.
 *
 * The layers are grids of 16-bit cell indices, MAP_W cells wide: 40 cells of
 * 8 pixels is the 320-pixel screen. The writers are compiled into every
 * overlay that draws a menu (src/common/gfx/tilemap.c, tilemapblit.c), and so
 * is the decimal formatter they are paired with (src/common/ui/decimal.c).
 *
 * A number goes down in two steps: FormatDecimal writes its digit bytes into
 * g_hud_digits, least significant first, and TileMapWriteRowRev lays them out
 * from a right-hand cell leftwards, adding the glyph of the font's zero.
 */
#include <decomp/types.h>

#define MAP_W 40

/* The second of the layers ADV clears; S2D's sits 0x20000 higher, which is
   what WORK_BIAS says. Reached by hardcoded address. */
#define g_tilemap1 ((short *)(0x800EF580 + WORK_BIAS))

/* The header layer: 40 cells by 32. */
#define g_tilemap2 ((short *)(0x800F0980 + WORK_BIAS))

/* Bank 0 of the font puts the ten digits at 0xC0..0xC9. */
#define GLYPH_DIGIT0 0xC0

/* The scratch FormatDecimal writes its digit bytes into, per overlay. */
extern u_char g_hud_digits[];

/* 0, 1, 2, 3...: with a base added, one ascending run draws any stretch of
   the font as a label. */
extern const u_char str_cell_run[];

extern void  TileMapWriteRow(const u_char *src, short *dst, int base,
                             u_short count);
extern void  TileMapWriteCol(const u_char *src, short *dst, int base,
                             u_short count, u_short stride);
/* dng's persona data page was built against int declarations of these:
   FormatDecimal's count goes over unnarrowed. */
#ifdef TILEMAP_INT_COUNT
extern void  TileMapWriteRowRev(const u_char *src, short *dst, int base,
                                int count);
#else
extern void  TileMapWriteRowRev(const u_char *src, short *dst, int base,
                                u_short count);
#endif
extern void  TileMapFillRect(short *dst, short value, u_short w, u_short h,
                             u_short stride);
#ifdef TILEMAP_INT_COUNT
extern int   FormatDecimal(u_int value, u_char *dst, u_short width);
#else
extern short FormatDecimal(u_int value, u_char *dst, u_short width);
#endif

#endif
