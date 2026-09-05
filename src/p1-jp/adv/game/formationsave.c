/* Persona 1 (JP) - the "save this layout?" prompt on the formation screen.
 *
 *   ADV @ 0x80074160.  S2D's copy reaches its own grid 0x20000 higher; see
 *   src/p1-jp/s2d/game/formationsave.c.
 *
 * The grid goes into the last row of g_formation_preset first and is copied
 * from there into the chosen slot, so the row the player is standing on and
 * the one they save are written by the same two loops.
 */
#include <types.h>
#include <persona/common/menuctx.h>
#include <persona/common/slot.h>

#define g_formation        ((u_char *)0x800EB34C)
#define g_formation_preset ((u_char *)0x801F2584)
#define GRID_CELLS   25
#define LIVE_ROW     8

#define PROMPT_LAST  0x2A
#define SCREEN_SLOT  0x2B

extern short  g_menu_subsel;
extern u_char g_menu_allow_hold;
extern u_char InputCheckAcceptA(u_char repeat);
extern u_char InputCheckAcceptB(u_char repeat);
extern void   SlotSetPos(u_char slot, int attr, short x, short y);

void FormationSavePrompt(void)
{
    u_char *grid;
    u_char *live;
    u_char  i;

    live = &g_formation_preset[LIVE_ROW * GRID_CELLS];
    grid = g_formation;
    if (MenuStepCursor(&g_menu->list[0])) {
        SlotSetPos(PROMPT_LAST, 0x23, 0x108, g_menu->list[0].cur * 16 + 0x5A);
    }
    if (InputCheckAcceptA(1)) {
        if (g_menu->list[0].cur == 0) {
            for (i = 0; i < GRID_CELLS; i++) {
                live[i] = grid[i];
            }
            grid = &g_formation_preset[g_menu->list[1].cur * GRID_CELLS];
            for (i = 0; i < GRID_CELLS; i++) {
                grid[i] = live[i];
            }
            g_menu_subsel = 0;
            return;
        }
    } else if (InputCheckAcceptB(1) == 0 && g_menu_allow_hold == 0) {
        return;
    }
    SlotClear(0x24);
    SlotClear(0x25);
    SlotClear(0x26);
    SlotClear(0x27);
    SlotClear(0x28);
    SlotClear(0x29);
    SlotClear(0x2A);
    SlotSetFlicker(SCREEN_SLOT, 1);
    g_menu_subsel--;
}
