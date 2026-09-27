/* ADV builds DNG's unit (src/dng/ui/skilldraw.c): against the narrow
   declarations, with its own layout, and with the glyph base of
   TileMapWriteRowRev narrowed too. */
#define SKILLDRAW_ADV
#define TILEMAP_SHORT_BASE
#include "../../dng/ui/skilldraw.c"
