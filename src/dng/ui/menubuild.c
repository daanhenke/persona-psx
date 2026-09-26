/* Persona 1 (JP) - laying the field menu's top page out.  DNG's copy.
 *   DNG 0x80078DF8   (ADV's is at 0x80069824)
 *
 * MenuTick's phase 0: the three tile layers are blanked, every slot freed,
 * the command cursor re-armed on the entry it was left on, and the page
 * drawn again. A unit of its own between MenuTick (menutick.c) and the menu
 * input poll (menupoll.c).
 */
#define SLOT_TAGGED_INTXY
#include <decomp/types.h>
#include <persona/common/menuctx.h>
#include <persona/common/slot.h>
#include <persona/common/tilemap.h>

#ifndef g_tilemap0
#define g_tilemap0 ((short *)0x800EE180)
#endif

/* The top page's own sprite in slot 0x2F; every sub-page clears it. */
extern u_char g_menu_top_def[];
extern u_char g_menu_blink;

extern void func_80092E5C(int);
extern void MenuScreenDraw(void);
extern int  func_80085AF4(void);
extern void func_80086614(void);

void MenuBuild(void)
{
    func_80092E5C(0);
    TileMapFillRect(g_tilemap0, 0, MAP_W, 0x40, MAP_W);
    TileMapFillRect(g_tilemap1, 0, MAP_W, 0x40, MAP_W);
    TileMapFillRect(g_tilemap2, 0, MAP_W, 0x20, MAP_W);
    SlotClearAll();
    MenuListInit(&g_menu->top, g_menu->top.cur, 0, 4,
                 MENU_CLICK_B | MENU_RIGHT_IS_NEXT | MENU_DOWN_IS_NEXT |
                     MENU_WRAP);
    SlotInitTagged(g_menu_top_def, 0x2F, 0x380, 0, 0);
    MenuScreenDraw();
    func_80085AF4();
    if (g_menu_blink == 0xFF) {
        func_80086614();
    }
}
