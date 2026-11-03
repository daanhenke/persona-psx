/* S2D uses the same preset-list routine with its own text and work area. */
/* This copy narrows the glyph bank at the row call. */
#define PRESET_GLYPH_BASE(bank) ((u_short)(bank))
#define str_preset_saved D_8009D21C
#include "../../dng/ui/formationpresets.c"
