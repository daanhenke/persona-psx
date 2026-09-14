#ifndef PERSONA_BTLP_GFX_H
#define PERSONA_BTLP_GFX_H

/* Persona 1 (JP) - getting artwork into memory and onto the GPU.
 *
 * BtlUnpack expands one packed run into a buffer. BtlBindGfx fixes a model's
 * artwork up in place and records it under a kind and an index. BtlUploadTim
 * puts a TIM into VRAM at a page and its CLUT rows at a slot, and answers
 * where the CLUT was read from.
 *
 * uploadtim.c defines BtlUploadTim with `y` as a short and narrows it itself.
 * Every caller was built against an int, which is what is declared here, so
 * uploadtim.c does not include this header: a short here would have each
 * caller narrow the argument a second time on its own side.
 */
#include <decomp/types.h>

extern void    BtlUnpack(u_char *dst, const u_char *src);
extern int     BtlBindGfx(u_int kind, int index, u_char **image);
extern u_long *BtlUploadTim(u_long *tim, int page, int slot, int abr, int y,
                            int nclut);

#endif
