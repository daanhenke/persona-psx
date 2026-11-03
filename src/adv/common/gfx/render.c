/* adv's object for the shared unit in src/common.
 *
 * Each overlay compiles its own: the work area differs (WORK_BIAS) and
 * splat resolves symlinks when it writes the linker script, so a link
 * here would collapse back to one shared object.
 */

/* ADV's RenderFrame leaves FrameHook out; its own frame calls it. */
#define RENDER_NO_HOOK

#include "../../../common/gfx/render.c"
