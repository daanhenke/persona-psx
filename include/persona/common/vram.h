#ifndef PERSONA_COMMON_VRAM_H
#define PERSONA_COMMON_VRAM_H

/* Persona 1 (JP) - wiping VRAM (src/common/gfx/vram.c).
 *
 * Clears a rectangle to black with the display masked off, bracketed by
 * VSyncs: the path for wiping a texture page before loading into it, not
 * anything done per frame. */
#include <decomp/types.h>

void VramClearRect(short x, short y, short w, short h);

#endif
