/* Persona 1 (JP) - picking a member to move on the formation screen.  DNG only.
 * DNG's copy of src/adv/game/formationpick.c.
 *   ADV 0x80073120
 *
 * The first step of rearranging the party: the cursor walks a list whose first
 * entry is "done" and whose others are the members. On a member it lifts them
 * off the grid - their cell is emptied and the grid cursor starts from it -
 * and moves on to placing them; on "done" it opens the "save this layout?"
 * prompt, three rows of it. Backing out puts the grid back as it was when the
 * screen opened, from the live row of the saved layouts.
 */
#define SLOT_SETPOS_INT
#define SLOT_TAGGED_INTXY
#define TILEMAP_INT_COUNT
#define PERSONAPAGE_DNG
#define FORMATION_INT
#define SLOT_INIT_INTXY
#define SLOT_FLICKER_INT
#include <decomp/types.h>

/* This unit's calls to SlotSetFlicker pass the slot unmasked. */
#define SLOT_FLICKER_INT

#include <persona/adv/moneybox.h>
#include <persona/common/formation.h>
#include <persona/common/menuctx.h>
#include <persona/common/slot.h>

#define GRID_CELLS 25
#define GRID_W     5
#define LIVE_ROW   8

/* The sprites this step puts up. */
#define HINT_SLOT      0xB
#define HINT2_SLOT     0xC
#define GRID_CUR_SLOT  0xF
#define MARK_SLOT      0x10
#define PROMPT_SLOT    0x24   /* three rows, then their three labels */
#define PROMPT_LABEL   0x27
#define PROMPT_CUR     0x2A

/* Tested unmasked, as everywhere in DNG's menus. */
extern int    InputCheckAcceptA(int repeat);
extern int    InputCheckAcceptB(int repeat);

extern u_char g_fm_hint_def[];
extern u_char g_fm_hint2_def[];
extern u_char g_fm_mark_def[];
extern u_char g_fm_grid_cur_def[];
extern u_char g_fm_prompt_cur_def[];
extern short  g_fm_mark_pos[][2];

void FormationPickMember(void)
{
    u_char *live;
    u_char *grid;
    u_char *cells;
    u_char  i;       /* the member's cell, then the restore's counter */
    /* Eight bytes of frame nothing reads, as BgReset has. */
    int     unused[2];

    live = &g_formation_preset[LIVE_ROW * GRID_CELLS];
    grid = g_formation;
    cells = g_formation_cell;
    DrawStatusHud();
    if (MenuStepCursor(&g_menu->list[0])) {
        FormationPlaceMarkers();
        FormationDrawMembers();
        if (g_menu->list[0].cur == 0) {
            SlotInitTagged(g_fm_hint_def, HINT_SLOT, 8, 0xD8, 0x48);
            SlotInitTagged(g_fm_hint2_def, HINT2_SLOT, 7, 0xD8, 0x48);
            SlotClear(MARK_SLOT);
        } else {
            SlotClear(HINT_SLOT);
            SlotClear(HINT2_SLOT);
            SlotSetFlicker(g_menu->list[0].cur + 0x1A, 1);
            SlotSetFlicker(g_menu->list[0].cur + 1, 1);
            SlotInitTagged(g_fm_mark_def, MARK_SLOT, 0x42,
                           g_fm_mark_pos[g_menu->list[0].cur][0],
                           g_fm_mark_pos[g_menu->list[0].cur][1]);
            SlotSetFlicker(MARK_SLOT, 1);
        }
    }

    if (InputCheckAcceptA(1)) {
        if (g_menu->list[0].cur == 0) {
            SlotClear(HINT_SLOT);
            SlotClear(HINT2_SLOT);
            SlotClear(GRID_CUR_SLOT);
            SlotClear(MARK_SLOT);
            MenuListInit(&g_menu->list[0], 0, 0, 1, 0x1E);
            SlotInitTagged(g_fm_hint_def, PROMPT_SLOT + 0, 0x24, 0x108, 0x48);
            SlotInitTagged(g_fm_hint_def, PROMPT_SLOT + 1, 0x24, 0x108, 0x58);
            SlotInitTagged(g_fm_hint_def, PROMPT_SLOT + 2, 0x24, 0x108, 0x68);
            SlotInitTagged(g_fm_hint2_def, PROMPT_LABEL + 0, 0x22, 0x108, 0x48);
            SlotInitTagged(g_fm_hint2_def, PROMPT_LABEL + 1, 0x22, 0x108, 0x58);
            SlotInitTagged(g_fm_hint2_def, PROMPT_LABEL + 2, 0x22, 0x108, 0x68);
            SlotSetAnim(PROMPT_LABEL + 0, 0, 0, 0, 0, 0x10, 0, 0);
            SlotSetAnim(PROMPT_LABEL + 1, 0, 0, 0, 0x30, 0, 0, 0);
            SlotSetAnim(PROMPT_LABEL + 2, 0, 0, 0, 0x60, 0, 0, 0);
            SlotInitTagged(g_fm_prompt_cur_def, PROMPT_CUR, 0x23, 0x108,
                           g_menu->list[0].cur * 16 + 0x5A);
            SlotSetFlicker(PROMPT_CUR, 1);
            FormationCompact();
            FormationRepair();
            g_menu_subsel += 2;
        } else {
            i = *(cells + g_menu->list[0].cur - 1);
            if (i == 0xFF) {
                i = FormationFirstFree();
            }
            grid[i] = 0xFF;
            MenuListInit(&g_menu->grid[0], i / GRID_W, 0, GRID_W - 1, 0x16);
            MenuListInit(&g_menu->grid[1], i % GRID_W, 0, GRID_W - 1, 0x1A);
            SlotInit(g_fm_grid_cur_def, GRID_CUR_SLOT, 4,
                     g_menu->grid[1].cur * 16 + 0xD8,
                     g_menu->grid[0].cur * 8 + 0x10);
            g_menu_subsel++;
        }
    } else if (InputCheckAcceptB(1) || g_menu_allow_hold) {
        if (g_menu->list[0].cur == 0) {
            /* Back out: the grid as it was when the screen opened. */
        restore:
            for (i = 0; i < GRID_CELLS; i++) {
                grid[i] = live[i];
            }
            FormationRepair();
            SlotClear(HINT_SLOT);
            SlotClear(HINT2_SLOT);
            SlotSetFlicker(1, 1);
            g_menu_subsel--;
        } else if (g_menu_allow_hold) {
            goto restore;
        } else {
            MenuListInit(&g_menu->list[0], 0, 0, g_party_last + 1, 0x1E);
            SlotInitTagged(g_fm_hint_def, HINT_SLOT, 8, 0xD8, 0x48);
            SlotInitTagged(g_fm_hint2_def, HINT2_SLOT, 7, 0xD8, 0x48);
            SlotClear(MARK_SLOT);
            FormationRepair();
        }
    }
}
