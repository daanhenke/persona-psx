/* s2d's object for the shared unit in src/common.
 *
 * Each overlay compiles its own: the work area differs (WORK_BIAS) and
 * splat resolves symlinks when it writes the linker script, so a link
 * here would collapse back to one shared object.
 */
/* VramClearRect is still asm (see the note there). */
#define RENDER_ASM_VRAMCLEARRECT     INCLUDE_ASM("s2d/nonmatchings/common/gfx/render", VramClearRect)

#include "../../../common/gfx/render.c"
