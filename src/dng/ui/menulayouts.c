/* Persona 1 (JP) - the equip screen's and the use list's frames.  DNG only.
 *   0x8008A370 EquipScreenLayout   0x8008A534 ItemUseLayout
 *
 * Windows, boxes and the bar columns the rows are written over. The use list
 * also puts the menu's backdrop and the party slots back, as MenuScreenDraw
 * does.
 */
#include <decomp/types.h>
#include <persona/common/tilemap.h>
#include <persona/adv/personapage.h>

extern u_short g_menu_bg_rle[];

extern void BgBoxShow(void);
extern void DrawStatusHud(void);
extern void DrawPartySlotStatus(int slot, int kind);

/* The cell the use list writes between its columns. */
#define CELL_DIVIDER 0x17

void EquipScreenLayout(void)
{
    short i;

    TileMapDrawWindow(AT(g_tilemap0, 8, 17), 0x16, 0xA, MAP_W);
    TileMapDrawBox(AT(g_tilemap0, 9, 18), 0x14, 8, MAP_W);
    TileMapDrawWindow(AT(g_tilemap0, 7, 2), 0xE, 0xB, MAP_W);
    for (i = 0; i < 7; i++) {
        TileMapWriteBar(AT(g_tilemap0, 10 + i, 3), 2);
        TileMapWriteBar(AT(g_tilemap0, 10 + i, 5), 10);
    }
    for (i = 0; i < 5; i++) {
        TileMapWriteBar(AT(g_tilemap0, 11 + i, 19), 3);
        TileMapWriteBar(AT(g_tilemap0, 11 + i, 22), 2);
        TileMapWriteBar(AT(g_tilemap0, 11 + i, 24), 2);
    }
    for (i = 0; i < 6; i++) {
        TileMapWriteBar(AT(g_tilemap0, 10 + i, 27), 5);
        TileMapWriteBar(AT(g_tilemap0, 10 + i, 32), 2);
        TileMapWriteBar(AT(g_tilemap0, 10 + i, 34), 3);
    }
}

void ItemUseLayout(void)
{
    short i;

    TileMapDrawWindow(AT(g_tilemap0, 0, 7), 0x1E, 0xA, MAP_W);
    TileMapDrawBox(AT(g_tilemap0, 1, 8), 0x1C, 8, MAP_W);
    TileMapBlitRle(g_menu_bg_rle, AT(g_tilemap0, 10, 0), MAP_W);
    for (i = 0; i < 6; i++) {
        TileMapWriteBar(AT(g_tilemap0, 2 + i, 9), 10);
        TileMapWriteBar(AT(g_tilemap0, 2 + i, 19), 2);
        *AT(g_tilemap0, 2 + i, 21) = CELL_DIVIDER;
        *AT(g_tilemap0, 2 + i, 22) = CELL_DIVIDER;
        TileMapWriteBar(AT(g_tilemap0, 2 + i, 23), 10);
        TileMapWriteBar(AT(g_tilemap0, 2 + i, 33), 2);
    }
    BgBoxShow();
    DrawStatusHud();
    DrawPartySlotStatus(0, 0);
    DrawPartySlotStatus(1, 0);
    DrawPartySlotStatus(2, 0);
    DrawPartySlotStatus(3, 0);
    DrawPartySlotStatus(4, 0);
}
