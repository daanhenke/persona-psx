/* s2d's object for the shared unit in src/common.
 *
 * Each overlay compiles its own: the work area differs (WORK_BIAS) and
 * splat resolves symlinks when it writes the linker script, so a link
 * here would collapse back to one shared object.
 */
#include <persona/s2d/advport.h>
#include "../../../common/ui/menumember.c"
