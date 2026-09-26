/* dng's object for the shared unit in src/common, built against DNG's
 * int-taking declarations.
 *
 * Each overlay compiles its own: the work area differs (WORK_BIAS) and
 * splat resolves symlinks when it writes the linker script, so a link
 * here would collapse back to one shared object.
 */
#define SLOT_SETPOS_INT
#define FORMATION_INT
#include "../../../common/game/formationload.c"
