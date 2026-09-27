/* dng's object for the shared unit in src/common. */
#include "../../../common/gfx/slotrender.c"

/* Still asm here rather than in the shared source: splat reads this file,
   not the one it includes, to learn which routines are not C yet. */
#ifndef NON_MATCHING
INCLUDE_ASM("dng/nonmatchings/common/gfx/slotrender", SlotsRenderAll);
#endif
