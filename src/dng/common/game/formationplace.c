/* dng's object for the shared unit in src/common.
 *
 * Each overlay compiles its own: the work area differs (WORK_BIAS) and splat
 * resolves symlinks when it writes the linker script, so a link here would
 * collapse back to one shared object.
 *
 * This overlay's copy was built against an int-taking SlotInitTagged, so it
 * hands the slot over unmasked and the callee narrows it. See slot.h.
 */
#define SLOT_TAGGED_INT
#include "../../../common/game/formationplace.c"
