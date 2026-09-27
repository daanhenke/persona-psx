/* dng's object for the shared unit in src/common.
 *
 * Each overlay compiles its own: the work area differs (WORK_BIAS) and
 * splat resolves symlinks when it writes the linker script, so a link
 * here would collapse back to one shared object.
 */

/* dng hands the member's slot over unmasked. */
#define SLOT_TAGGED_INT
#define SLOT_CLEAR_INT

#include "../../../common/game/formationmembers.c"
