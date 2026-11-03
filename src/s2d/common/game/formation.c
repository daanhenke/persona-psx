/* s2d's object for the shared unit in src/common.
 *
 * Each overlay compiles its own: the work area differs (WORK_BIAS) and
 * splat resolves symlinks when it writes the linker script, so a link
 * here would collapse back to one shared object.
 */
/* Reuse the verified placement rule with S2D's work-area bias. */
#define FORMATION_CELL_FREE

#include "../../../common/game/formation.c"
