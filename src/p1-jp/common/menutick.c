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

/* g_menu_subsel doubles as the phase. MenuPollInput sets it back to 0 when
   accept is pressed, which is what makes the screen rebuild on the next tick.
 *
 * Written as a switch rather than an if/else chain: gcc 2.6 expands the two
 * cases into `beqz .build / beq 1 .poll / j .out`, keeping the second compare
 * that an if/else collapses. */
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
