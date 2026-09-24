/* dng's object for the shared unit in src/common.
 *
 * Each overlay compiles its own: the work area differs (WORK_BIAS) and
 * splat resolves symlinks when it writes the linker script, so a link
 * here would collapse back to one shared object.
 */
#include "../../../common/gfx/fontglyph.c"

/* Still asm here rather than in the shared source: splat reads this file,
   not the one it includes, to learn which routines are not C yet. */
#ifndef NON_MATCHING
INCLUDE_ASM("dng/nonmatchings/common/gfx/fontglyph", FontUploadGlyph);
#endif
