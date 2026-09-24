/* Persona 1 (JP) - placing a member on the formation grid.  ADV only.
 *   0x800736D4 FormationPlaceMember   0x800739CC FormationDonePrompt
 *   0x80073D10 FormationPresetPick
 *
 * The member lifted by FormationPickMember is carried around the grid by a
 * cursor. Dropping them on another member swaps the two; dropping them on a
 * free cell moves them there. "Done" then asks whether to keep the layout:
 * the first answer files it under one of the eight saved layouts, the second
 * just makes it the live one.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>

/* This unit's calls to SlotSetFlicker pass the slot unmasked. */
#define SLOT_FLICKER_INT

#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <persona/common/menuctx.h>
#include <persona/common/formation.h>
#include <persona/adv/personapage.h>

#define LIVE_ROW 8
#define PRESETS  8

#define GRID_CUR_SLOT      0xF
#define PROMPT_SLOT        0x24   /* three rows, then their three labels */
#define PROMPT_CUR         0x2A
#define PRESET_CURSOR_SLOT 0x2B

extern short   g_menu_subsel;
extern u_char  g_fm_hint_def[];
extern u_char  g_fm_hint2_def[];
extern u_char  g_fm_prompt_cur_def[];
extern int     g_pad_held[];

/* L1 and R1. */
#define PAD_SHOULDERS 0xA000

void FormationMenuOpen(void);

extern void DrawStatusHud(void);
extern void FormationDrawPresets(void);

/* 98.96%: the same code, but the global allocator ranks the member's target
   cell last where the image gives it s0 - pinning it there leaves only the
   computation temporaries different. Something in the original gave `cell`
   more references or a shorter life at no cost in code. */
#ifdef NON_MATCHING
void FormationPlaceMember(void)
{
    u_char *grid = g_formation;
    u_char *cells = g_formation_cell;
    u_char  member = g_menu->list[0].cur - 1;
    u_char  cell;
    u_char  other;

    DrawStatusHud();
    if (!MenuStepCursor(&g_menu->grid[0])) {
        MenuStepCursor(&g_menu->grid[1]);
    }
    SlotSetPos(GRID_CUR_SLOT, 4, g_menu->grid[1].cur * 16 + 0xD8,
               g_menu->grid[0].cur * 8 + 0x10);
    FormationPlaceMarkers();
    FormationDrawMembers();
    SlotSetFlicker(g_menu->list[0].cur + 0x1A, 1);
    SlotSetFlicker(g_menu->list[0].cur + 1, 1);
    cell = g_menu->grid[0].cur * GRID_W + g_menu->grid[1].cur;
    other = FormationOtherAt(member, cell);
    if (other != CELL_EMPTY) {
        SlotSetFlicker(other + 2, 1);
    }
    if (InputCheckAcceptA(1)) {
        other = FormationOtherAt(member, cell);
        if (other != CELL_EMPTY) {
            u_char was = cells[member];

            cells[member] = cells[other];
            cells[other] = was;
            grid[cell] = member;
            grid[cells[other]] = other;
        } else if (FormationCellFree(cell)) {
            grid[cells[member]] = CELL_EMPTY;
            cells[member] = cell;
            grid[cell] = member;
        } else {
            /* A cell nobody can stand on: stay put. */
            goto keep;
        }
        FormationPlaceMarkers();
        FormationDrawMembers();
        SlotClear(GRID_CUR_SLOT);
        g_menu_subsel--;
    keep:
        SlotSetFlicker(g_menu->list[0].cur + 0x1A, 1);
        SlotSetFlicker(g_menu->list[0].cur + 1, 1);
    } else if (InputCheckAcceptB(1) || g_menu_allow_hold) {
        grid[cells[member]] = member;
        FormationPlaceMarkers();
        FormationDrawMembers();
        SlotSetFlicker(g_menu->list[0].cur + 0x1A, 1);
        SlotSetFlicker(g_menu->list[0].cur + 1, 1);
        SlotClear(GRID_CUR_SLOT);
        g_menu_subsel--;
    }
}
#else
INCLUDE_ASM("adv/nonmatchings/game/formationplace", FormationPlaceMember);
#endif

void FormationDonePrompt(void)
{
    u_char *live = &g_formation_preset[LIVE_ROW * GRID_CELLS];
    u_char *grid = g_formation;
    u_char  i;

    DrawStatusHud();
    if (MenuStepCursor(&g_menu->list[0])) {
        SlotSetPos(PROMPT_CUR, 0x23, 0x108, g_menu->list[0].cur * 16 + 0x5A);
    }
    FormationPlaceMarkers();
    FormationDrawMembers();
    if (InputCheckAcceptA(1)) {
        SlotClear(PROMPT_SLOT);
        SlotClear(PROMPT_SLOT + 1);
        SlotClear(PROMPT_SLOT + 2);
        SlotClear(PROMPT_SLOT + 3);
        SlotClear(PROMPT_SLOT + 4);
        SlotClear(PROMPT_SLOT + 5);
        SlotClear(PROMPT_CUR);
        if (g_menu->list[0].cur == 0) {
            func_8008EDBC(0x12);
            TileMapFillRect(g_tilemap0, 0, MAP_W, 0x40, MAP_W);
            TileMapFillRect(g_tilemap1, 0, MAP_W, 0x40, MAP_W);
            TileMapDrawWindow(g_tilemap0, 0xE, 0xC, MAP_W);
            TileMapDrawBox(AT(g_tilemap0, 1, 1), 0xC, 0xA, MAP_W);
            for (i = 0; i < PRESETS; i++) {
                TileMapWriteBar(AT(g_tilemap0, i + 2, 2), 10);
            }
            FormationDrawPresets();
            MenuListInit(&g_menu->list[1], 0, 0, PRESETS - 1, 0x16);
            SlotInitTagged(g_pdata_cursor_def, PRESET_CURSOR_SLOT, 0x42, 0x60,
                           g_menu->list[1].cur * 12 + 0x80);
            SlotSetFlicker(PRESET_CURSOR_SLOT, 1);
            g_menu_subsel++;
        } else {
            for (i = 0; i < GRID_CELLS; i++) {
                live[i] = grid[i];
            }
            g_menu_subsel = 1;
        }
    } else if (InputCheckAcceptB(1) || g_menu_allow_hold) {
        MenuListInit(&g_menu->list[0], 0, 0, g_party_last + 1, 0x1E);
        SlotInitTagged(g_fm_hint_def, 0xB, 8, 0xD8, 0x48);
        SlotInitTagged(g_fm_hint2_def, 0xC, 7, 0xD8, 0x48);
        SlotClear(PROMPT_SLOT);
        SlotClear(PROMPT_SLOT + 1);
        SlotClear(PROMPT_SLOT + 2);
        SlotClear(PROMPT_SLOT + 3);
        SlotClear(PROMPT_SLOT + 4);
        SlotClear(PROMPT_SLOT + 5);
        SlotClear(PROMPT_CUR);
        g_menu_subsel -= 2;
    }
}

/* Picking the saved layout to file the grid under, a frame. The cursor lays
   each layout out on the grid as it passes, or, while a shoulder button is
   held, shows the grid as it stands. Accepting asks whether to overwrite it;
   backing out returns to the "done" prompt. */
void FormationPresetPick(void)
{
    if (MenuStepCursor(&g_menu->list[1])) {
        SlotSetPos(PRESET_CURSOR_SLOT, 0x42, 0x60,
                   g_menu->list[1].cur * 12 + 0x80);
    }
    if (g_pad_held[0] & PAD_SHOULDERS) {
        FormationRepair();
    } else {
        FormationLoadPreset(g_menu->list[1].cur);
    }
    if (InputCheckAcceptA(1)) {
        FormationLoadPreset(g_menu->list[1].cur);
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
    } else if (InputCheckAcceptB(1) || g_menu_allow_hold) {
        FormationMenuOpen();
        MenuListInit(&g_menu->list[0], 0, 0, 1, 0x1E);
        SlotClear(PRESET_CURSOR_SLOT);
        SlotInitTagged(g_fm_hint_def, PROMPT_SLOT, 0x24, 0x108, 0x48);
        SlotInitTagged(g_fm_hint_def, PROMPT_SLOT + 1, 0x24, 0x108, 0x58);
        SlotInitTagged(g_fm_hint_def, PROMPT_SLOT + 2, 0x24, 0x108, 0x68);
        SlotInitTagged(g_fm_hint2_def, PROMPT_SLOT + 3, 0x22, 0x108, 0x48);
        SlotInitTagged(g_fm_hint2_def, PROMPT_SLOT + 4, 0x22, 0x108, 0x58);
        SlotInitTagged(g_fm_hint2_def, PROMPT_SLOT + 5, 0x22, 0x108, 0x68);
        SlotSetAnim(PROMPT_SLOT + 3, 0, 0, 0, 0, 0x10, 0, 0);
        SlotSetAnim(PROMPT_SLOT + 4, 0, 0, 0, 0x30, 0, 0, 0);
        SlotSetAnim(PROMPT_SLOT + 5, 0, 0, 0, 0x60, 0, 0, 0);
        SlotInitTagged(g_fm_prompt_cur_def, PROMPT_CUR, 0x23, 0x108,
                       g_menu->list[0].cur * 16 + 0x5A);
        SlotSetFlicker(1, 0);
        SlotSetFlicker(PROMPT_CUR, 1);
        FormationSyncCells();
        g_menu_subsel--;
    }
}
