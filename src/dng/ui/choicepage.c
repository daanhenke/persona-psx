/* Persona 1 (JP) - a page of three choices over two columns.  DNG only.
 *   0x80085254 MenuChoicePageOpen
 *
 * A window with three six-cell headings, seven rows of two ten-cell fields
 * under them, and the cursor put on the heading menuctx's unk1F0 is on (the
 * main menu arms that list with three stops). No call reaches it and no
 * JP1 file holds its address as a word, so if a table leads here it is one
 * filled at run time.
 */
#define SLOT_TAGGED_INTXY
#define SLOT_SETPOS_INT
#include <decomp/types.h>
#include <persona/common/menuctx.h>
#include <persona/common/slot.h>
#include <persona/common/tilemap.h>
#include <persona/adv/personapage.h>

extern u_char g_fm_prompt_cur_def[];
extern u_char D_8009AA4C[];
extern u_char D_8009B074[];

extern void MenuSetLayers(int);

void MenuChoicePageOpen(void)
{
    int i;

    MenuSetLayers(0x11);
    TileMapFillRect(g_tilemap0, 0, MAP_W, 0x40, MAP_W);
    TileMapFillRect(g_tilemap1, 0, MAP_W, 0x40, MAP_W);
    TileMapDrawWindow(AT(g_tilemap0, 0, 3), 0x1A, 0xD, MAP_W);
    TileMapWriteBar(AT(g_tilemap0, 2, 6), 6);
    TileMapWriteBar(AT(g_tilemap0, 2, 13), 6);
    TileMapWriteBar(AT(g_tilemap0, 2, 20), 6);
    for (i = 0; i < 7; i++) {
        TileMapWriteBar(AT(g_tilemap0, 4 + i, 5), 10);
        TileMapWriteBar(AT(g_tilemap0, 4 + i, 17), 10);
    }
    SlotClearAll();
    SlotInitTagged(D_8009AA4C, 0x3C, 0x300, 0x18, 0x18);
    SlotInitTagged(D_8009B074, 0x2D, 0x2FF, 0, 0x10);
    SlotSetAnim(0x2D, 0, 0, 0, 0x30, 0x24, 0, 0);
    SlotInitTagged(g_fm_prompt_cur_def, 1, 0x42, 0, 0);
    SlotSetPos(1, 0x42, g_menu->unk1F0.cur * 56 + 0x60, 0x24);
    SlotSetFlicker(1, 1);
}
