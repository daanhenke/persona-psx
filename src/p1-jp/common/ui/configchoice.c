/* Persona 1 (JP) - two states of the config screen.
 *
 * Compiled into two overlays rather than called across the boundary:
 *   ADV @ 0x80075DB0 / 0x80075E94   S2D @ 0x80074D68 / 0x80074E4C
 *
 * The screen runs as nine states in g_menu_subsel. These are the two that
 * handle the row of choices along the top: state 7 walks a cursor across it,
 * moving the highlight slot 56 pixels a step, and opening a window below when
 * a choice is taken; state 8 is that window, and closes it again on cancel.
 *
 * The cursor's own position lives in the menu block at +0x1F0 rather than in a
 * global, which is why both go through the block pointer.
 */
#include <types.h>

#define g_tilemap0 ((short *)(0x800EE180 + WORK_BIAS))
#define MAP_W 40

extern u_char  *g_menu;
extern char     g_menu_allow_hold;
extern u_short  g_menu_subsel;

extern int  MenuStepCursor(int *cursor);
extern void SlotSetPos(u_char slot, int attr, short x, short y);
extern char InputCheckAcceptA(short pad);
extern char InputCheckAcceptB(short pad);
extern void SlotSetFlicker(u_char slot, u_char on);
extern void TileMapDrawWindow(short *dst, u_short w, u_short h, u_short stride);
extern void TileMapFillRect(short *dst, short value, u_short w, u_short h,
                            u_short stride);

#define CHOICE_CURSOR 0x1F0    /* offset of this row's cursor in the block */
#define CURSOR_SLOT   1
#define CURSOR_Z      0x42
#define CHOICE_PITCH  0x38     /* pixels between choices                   */
#define CHOICE_X0     0x60
#define CHOICE_Y      0x24
#define WINDOW_ROW    13

void ConfigStepChoice(void)
{
    if (MenuStepCursor((int *)(g_menu + CHOICE_CURSOR)) != 0) {
        SlotSetPos(CURSOR_SLOT, CURSOR_Z,
                   *(int *)(g_menu + CHOICE_CURSOR) * CHOICE_PITCH + CHOICE_X0,
                   CHOICE_Y);
    }
    if (InputCheckAcceptA(1)) {
        TileMapDrawWindow(&g_tilemap0[WINDOW_ROW * MAP_W], 0x20, 6, MAP_W);
        SlotSetFlicker(CURSOR_SLOT, 0);
        g_menu_subsel++;
    } else if (InputCheckAcceptB(1) || g_menu_allow_hold) {
        g_menu_subsel = 0;
    }
}

void ConfigCloseChoice(void)
{
    if (InputCheckAcceptB(1) || g_menu_allow_hold) {
        TileMapFillRect(&g_tilemap0[WINDOW_ROW * MAP_W], 0, 0x28, 6, MAP_W);
        SlotSetFlicker(CURSOR_SLOT, 1);
        g_menu_subsel--;
    }
}
