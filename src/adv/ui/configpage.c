/* Persona 1 (JP) - opening the config page.  ADV only.
 *   0x80075F14 ConfigPageOpen
 *
 * The config page's window, its labels, the row cursor and the two option
 * markers; the rows' values are drawn by the list helpers in config.c.
 */
#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <persona/common/menuctx.h>
#include <persona/adv/personapage.h>

extern u_char D_800B1D08[];
extern u_char D_800B2330[];
extern u_char g_config_marker_def[];

extern void TileMapWriteRun10(short *dst);
extern void ConfigDrawLabels(void);
extern void ConfigListBeginEdit(void);
extern void ConfigListPlaceMarkers(void);

void ConfigPageOpen(void)
{
    func_8008EDBC(0xE);
    TileMapFillRect(g_tilemap0, 0, MAP_W, 0x40, MAP_W);
    TileMapFillRect(g_tilemap1, 0, MAP_W, 0x40, MAP_W);
    TileMapDrawWindow(g_tilemap0, 0x1D, 0xF, MAP_W);
    TileMapDrawBox(AT(g_tilemap0, 1, 1), 0x1B, 0xD, MAP_W);
    TileMapWriteRun10(AT(g_tilemap0, 2, 2));
    ConfigDrawLabels();
    SlotClearAll();
    SlotInitTagged(D_800B1D08, 0x3C, 0x300, 0x18, 0x18);
    SlotInitTagged(D_800B2330, 0x2D, 0x2FF, 0, 0x10);
    SlotSetAnim(0x2D, 0, 0, 0, 0, 0x24, 0, 0);
    SlotInitTagged(g_pdata_cursor_def, 1, 0x42, 0x48,
                   g_menu->cfg_list.cur * 24 + 0x48);
    SlotInitTagged(g_config_marker_def, 2, 0x42, 0, 0);
    SlotInitTagged(g_config_marker_def, 3, 0x42, 0, 0);
    SlotSetFlicker(1, 1);
    ConfigListBeginEdit();
    ConfigListPlaceMarkers();
}
