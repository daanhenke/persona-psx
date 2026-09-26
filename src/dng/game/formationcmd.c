/* Persona 1 (JP) - the formation screen's command prompt.  DNG only.
 * DNG's copy of src/adv/game/formationcmd.c.
 *   0x80072D7C FormationCmdStep
 *
 * The first step of the formation screen: arrange the party by hand, or pick
 * one of the eight saved layouts. Arranging starts the member cursor on
 * "done"; the layouts open their list with the first one laid out on the
 * grid if it fits the party. Backing out returns to the menu's top level.
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

#define CMD_CURSOR_SLOT 1
#define PRESET_CURSOR_SLOT 0x2B
#define MARK_SLOT 0x10
#define PRESETS 8

extern short   g_menu_subsel;
extern short   g_menu_sel;
extern u_char  g_menu_blink;
extern u_char  g_fm_hint_def[];
extern u_char  g_fm_mark_def[];
extern short   g_fm_mark_pos[][2];

extern void DrawStatusHud(void);
extern void FormationDrawPresets(void);

void FormationCmdStep(void)
{
    u_char i;
    /* Eight bytes of frame nothing reads, as FormationPickMember has. */
    int    unused[2];

    DrawStatusHud();
    if (MenuStepCursor(&g_menu->formation_cmd)) {
        SlotSetPos(CMD_CURSOR_SLOT, 0x23, 6,
                   g_menu->formation_cmd.cur * 16 + 0x32);
    }
    if (InputCheckAcceptA(1)) {
        SlotSetFlicker(CMD_CURSOR_SLOT, 0);
        if (g_menu->formation_cmd.cur == 0) {
            MenuListInit(&g_menu->list[0], g_party_last + 1, 0,
                         g_party_last + 1, 0x1E);
            SlotInitTagged(g_fm_hint_def, 0xB, 8, 0xD8, 0x48);
            SlotClear(0xB);
            SlotClear(0xC);
            SlotSetFlicker(g_menu->list[0].cur + 0x1A, 1);
            SlotSetFlicker(g_menu->list[0].cur + 1, 1);
            SlotInitTagged(g_fm_mark_def, MARK_SLOT, 0x42,
                           g_fm_mark_pos[g_menu->list[0].cur][0],
                           g_fm_mark_pos[g_menu->list[0].cur][1]);
            SlotSetFlicker(MARK_SLOT, 1);
            g_menu_subsel++;
        } else {
            func_80092E5C(0x12);
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
            if (FormationPresetFits(g_menu->list[1].cur)) {
                FormationLoadPreset(g_menu->list[1].cur);
                FormationDrawMembers();
            } else {
                FormationClearMarkers();
            }
            SlotSetFlicker(PRESET_CURSOR_SLOT, 1);
            g_menu_subsel += 6;
        }
    } else if (InputCheckAcceptB(1) || g_menu_allow_hold) {
        g_menu_blink = 0xFF;
        g_menu_sel = 0;
        g_menu_subsel = 0;
    }
    /* The top menu's hand, kept on its row. */
    g_slot_cur = g_slots;
    g_slot_cur->y = g_menu->unk170.cur * 16 + 0x20;
}
