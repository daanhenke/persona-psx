/* Persona 1 (JP) - the config page's rows.  DNG only.
 * DNG's copy of src/adv/ui/configrows.c.
 *   0x8007584C ConfigStepRows
 *
 * A frame of the config page: the row cursor, and the option cursor on the
 * row. Accepting the auto-battle row opens the tactics page: each member's
 * name and the three tactics, with a marker on the one set for them.
 */
#define SLOT_SETPOS_INT
#define SLOT_SETPOS_SLOT_INT
#define SLOT_TAGGED_INTXY
#define SLOT_TAGGED_INT
#define TILEMAP_INT_COUNT
#define PERSONAPAGE_DNG
#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <persona/common/menuctx.h>
#include <persona/common/char.h>
#include <persona/common/formation.h>
#include <persona/adv/personapage.h>

#define ROW_AUTO_BATTLE 3

/* Each member's tactic, by party slot, in the save-game options. */
#define g_party_tactic ((u_char *)0x801F2AE6)

extern short  g_menu_subsel;
extern u_char str_cfg_auto_battle[];
extern u_char D_8009A5E0[];
extern u_char g_config_marker_def[];

extern void TileMapWriteRun10(short *dst);
extern void ConfigBeginEdit(void);
extern void ConfigApplyOption(void);
extern void ConfigPlaceMarkers(void);
extern void ConfigPageOpen(void);

void ConfigStepRows(void)
{
    int i;

    if (MenuStepCursor(&g_menu->cfg_row)) {
        SlotSetPos(1, 0x42, 0x48, g_menu->cfg_row.cur * 24 + 0x48);
        ConfigBeginEdit();
    } else if (MenuStepCursor(&g_menu->list[1])) {
        ConfigApplyOption();
    }
    ConfigPlaceMarkers();
    if (InputCheckAcceptA(0)) {
        if (g_menu->cfg_row.cur == ROW_AUTO_BATTLE) {
            TileMapFillRect(g_tilemap1, 0, MAP_W, 0x40, MAP_W);
            TileMapDrawWindow(g_tilemap0, 0x1D, 0xF, MAP_W);
            TileMapDrawBox(AT(g_tilemap0, 1, 1), 0x1B, 0xD, MAP_W);
            TileMapWriteRun10(AT(g_tilemap0, 2, 2));
            TileMapWriteRow(str_cfg_auto_battle, AT(g_tilemap1, 1, 0), 0, 10);
            for (i = 0; i <= g_party_last; i++) {
                TileMapWriteRow(g_chars[g_party_at[i]].name,
                                AT(g_tilemap1, 3 + i * 2, 1), 0, 8);
                TileMapWriteRow(D_8009A5E0, AT(g_tilemap1, 3 + i * 2, 11), 0,
                                4);
                TileMapWriteRow(D_8009A5E0 + 4, AT(g_tilemap1, 3 + i * 2, 16),
                                0, 4);
                TileMapWriteRow(D_8009A5E0 + 8, AT(g_tilemap1, 3 + i * 2, 21),
                                0, 4);
            }
            i = g_menu->member_list.cur;
            MenuListInit(&g_menu->list[1], g_party_tactic[i], 0, 2, 0x1A);
            SlotSetPos(1, 0x42, 0x48, g_menu->member_list.cur * 24 + 0x48);
            SlotClear(2);
            SlotClear(3);
            SlotClear(4);
            for (i = 0; i <= g_party_last; i++) {
                SlotInitTagged(g_config_marker_def, i + 2, 0x42, 0, 0);
                SlotSetPos(i + 2, 0x42, g_party_tactic[i] * 40 + 0xA0,
                           i * 24 + 0x48);
            }
            g_menu_subsel++;
        }
    } else if (InputCheckAcceptB(1) || g_menu_allow_hold) {
        ConfigPageOpen();
        g_menu_subsel -= 3;
    }
}
