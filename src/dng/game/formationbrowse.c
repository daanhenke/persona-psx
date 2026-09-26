/* Persona 1 (JP) - browsing the saved formations to load one.  DNG only.
 * DNG's copy of src/adv/game/formationbrowse.c.
 *   0x80074318 FormationLoadPick
 *
 * The cursor lays each saved layout out on the grid as it passes, when it
 * fits the party, and clears the markers when it does not; holding a
 * shoulder button shows the grid as it stands. Accepting a layout that fits
 * asks whether to load it. Backing out puts the live layout back.
 */
#define SLOT_SETPOS_INT
#define SLOT_TAGGED_INTXY
#define TILEMAP_INT_COUNT
#define PERSONAPAGE_DNG
#define FORMATION_INT
#include <decomp/types.h>

/* This unit's calls to SlotSetFlicker pass the slot unmasked. */
#define SLOT_FLICKER_INT

#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <persona/common/menuctx.h>
#include <persona/common/formation.h>
#include <persona/adv/personapage.h>

#define LIVE_ROW 8

#define GRID_CUR_SLOT      0xF
#define PROMPT_SLOT        0x24   /* three rows, then their three labels */
#define PROMPT_CUR         0x2A
#define PRESET_CURSOR_SLOT 0x2B

/* L1 and R1. */
#define PAD_SHOULDERS 0xA000

extern short   g_menu_subsel;
extern int     g_pad_held[];
extern u_char  g_fm_hint_def[];
extern u_char  g_fm_hint2_def[];
extern u_char  g_fm_prompt_cur_def[];

extern void FormationMenuOpen(void);

void FormationLoadPick(void)
{
    u_char *live = &g_formation_preset[LIVE_ROW * GRID_CELLS];
    u_char *grid = g_formation;
    short   i;

    if (MenuStepCursor(&g_menu->list[1])) {
        SlotSetPos(PRESET_CURSOR_SLOT, 0x42, 0x60,
                   g_menu->list[1].cur * 12 + 0x80);
    }
    if (g_pad_held[0] & PAD_SHOULDERS) {
        FormationRepair();
        FormationDrawMembers();
    } else if (FormationPresetFits(g_menu->list[1].cur)) {
        FormationLoadPreset(g_menu->list[1].cur);
        FormationDrawMembers();
    } else {
        FormationClearMarkers();
    }
    if (InputCheckAcceptA(1)) {
        if ((short)FormationPresetFits(g_menu->list[1].cur) == 1) {
            FormationLoadPreset(g_menu->list[1].cur);
            FormationDrawMembers();
            MenuListInit(&g_menu->list[0], 0, 0, 1, 0x1E);
            SlotInitTagged(g_fm_hint_def, PROMPT_SLOT, 0x24, 0x108, 0x48);
            SlotInitTagged(g_fm_hint_def, PROMPT_SLOT + 1, 0x24, 0x108, 0x58);
            SlotInitTagged(g_fm_hint_def, PROMPT_SLOT + 2, 0x24, 0x108, 0x68);
            SlotInitTagged(g_fm_hint2_def, PROMPT_SLOT + 3, 0x22, 0x108, 0x48);
            SlotInitTagged(g_fm_hint2_def, PROMPT_SLOT + 4, 0x22, 0x108, 0x58);
            SlotInitTagged(g_fm_hint2_def, PROMPT_SLOT + 5, 0x22, 0x108, 0x68);
            SlotSetAnim(PROMPT_SLOT + 4, 0, 0, 0, 0x30, 0, 0, 0);
            SlotSetAnim(PROMPT_SLOT + 5, 0, 0, 0, 0x60, 0, 0, 0);
            SlotInitTagged(g_fm_prompt_cur_def, PROMPT_CUR, 0x23, 0x108,
                           g_menu->list[0].cur * 16 + 0x5A);
            SlotSetFlicker(PROMPT_CUR, 1);
            SlotSetFlicker(PRESET_CURSOR_SLOT, 0);
            g_menu_subsel++;
        }
    } else if (InputCheckAcceptB(1) || g_menu_allow_hold) {
        FormationMenuOpen();
        MenuListInit(&g_menu->list[0], 1, 0, 1, 0x1E);
        SlotSetFlicker(1, 1);
        SlotClear(GRID_CUR_SLOT);
        for (i = 0; i < GRID_CELLS; i++) {
            grid[i] = live[i];
        }
        FormationRepair();
        g_menu_subsel -= 6;
    }
}
