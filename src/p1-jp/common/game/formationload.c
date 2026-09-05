/* Persona 1 (JP) - the "load this layout?" prompt on the formation screen.
 *
 *   ADV 0x800746C0   S2D 0x80073678
 *
 * Everything it touches is either the shared save-game work area or a menu
 * global the linker resolves per overlay, so one source covers both. The
 * saving half has to reach the live grid and does not; see
 * src/p1-jp/adv/game/formationsave.c.
 */
#include <types.h>
#include <persona/common/menuctx.h>
#include <persona/common/slot.h>

/* Nine rows of 25 cells: eight saved layouts and, last, the one in play. */
#define g_formation_preset ((u_char *)0x801F2584)
#define GRID_CELLS   25
#define LIVE_ROW     8

#define PROMPT_FIRST 0x24   /* the prompt's own sprites, cleared either way */
#define PROMPT_LAST  0x2A
#define SCREEN_SLOT  0x2B

extern short  g_menu_subsel;
extern u_char g_menu_allow_hold;
extern u_char InputCheckAcceptA(u_char repeat);
extern u_char InputCheckAcceptB(u_char repeat);
extern void   SlotSetPos(u_char slot, int attr, short x, short y);

void FormationLoadPrompt(void)
{
    u_char *live;
    u_char *row;
    u_char  i;

    live = &g_formation_preset[LIVE_ROW * GRID_CELLS];
    row = &g_formation_preset[g_menu->list[1].cur * GRID_CELLS];
    if (MenuStepCursor(&g_menu->list[0])) {
        SlotSetPos(PROMPT_LAST, 0x23, 0x108, g_menu->list[0].cur * 16 + 0x5A);
    }
    if (InputCheckAcceptA(1)) {
        if (g_menu->list[0].cur == 0) {
            for (i = 0; i < GRID_CELLS; i++) {
                live[i] = row[i];
            }
            SlotClear(0x24);
            SlotClear(0x25);
            SlotClear(0x26);
            SlotClear(0x27);
            SlotClear(0x28);
            SlotClear(0x29);
            SlotClear(0x2A);
            g_menu_subsel = 0;
            return;
        }
    } else {
        if (InputCheckAcceptB(1) == 0 && g_menu_allow_hold == 0) {
            return;
        }
        SlotSetFlicker(SCREEN_SLOT, 1);
    }
    SlotClear(0x24);
    SlotClear(0x25);
    SlotClear(0x26);
    SlotClear(0x27);
    SlotClear(0x28);
    SlotClear(0x29);
    SlotClear(0x2A);
    g_menu_subsel--;
}
