/* Persona 1 (JP) - putting the menu's top page back.  ADV only.
 *   0x800768F0 MenuTopRedraw
 *
 * A sub-page closing back to the command row: the three tile layers blanked,
 * every slot freed, the backdrop and party redrawn, and the row's own three
 * sprites put up again, with the map scroll back at the top.
 */
#include <decomp/types.h>
#include <persona/common/slot.h>
#include <persona/common/tilemap.h>
#include <persona/adv/personapage.h>

/* The top page's own sprite in slot 0x2F; every sub-page clears it. */
extern u_char g_menu_top_def[];
extern u_char D_800B1D08[];
extern u_char D_800B2330[];

extern short g_cam_y;
extern short g_map_scroll_y;

extern void MenuSetLayers(int);
extern void MenuScreenDraw(void);

void MenuTopRedraw(void)
{
    MenuSetLayers(0);
    SlotClearAll();
    TileMapFillRect(g_tilemap0, 0, MAP_W, 0x40, MAP_W);
    TileMapFillRect(g_tilemap1, 0, MAP_W, 0x40, MAP_W);
    TileMapFillRect(g_tilemap2, 0, MAP_W, 0x40, MAP_W);
    MenuScreenDraw();
    SlotInitTagged(D_800B1D08, 0x3C, 0x300, 0xA0, 0x48);
    SlotInitTagged(D_800B2330, 0x2D, 0x2FF, 0x88, 0x41);
    SlotInitTagged(g_menu_top_def, 0x2F, 0x380, 0, 0);
    g_cam_y = 0;
    g_map_scroll_y = 0;
}
