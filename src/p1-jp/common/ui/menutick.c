/* Persona 1 (JP) - menu screen driver.
 *
 * Compiled into three overlays rather than called across the boundary:
 *   DNG @ 0x80078D88
 *   ADV @ 0x800697B4
 *   S2D @ 0x80068D40
 * Each overlay has its own copy of the menu globals at its own addresses, so
 * one source covers all three through the per-target linker scripts.
 */
#include <types.h>
#include <persona/common/menu.h>

extern void MenuBuild(void);

/* One frame of the menu, called from the overlay's main loop. Phase 0 lays the
   screen out and arms the cursor blink for 0x20 frames; phase 1 is the steady
   state that polls input. Any other value does nothing, so the caller can park
   the menu by writing one.

   g_menu_subsel doubles as that phase: MenuPollInput sets it back to 0 when
   accept is pressed, which is what makes the screen rebuild on the next tick. */
void MenuTick(void)
{
    switch (g_menu_subsel) {
    case 0:
        MenuBuild();
        g_menu_blink = 0x20;
        g_menu_subsel++;
        break;
    case 1:
        MenuPollInput();
        break;
    }
}
