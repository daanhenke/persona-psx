/* Persona 1 (JP) - the "other" and config menus.  ADV only.
 *   0x80074870 ConfigMenuStep   0x80074954 ConfigMenuOpen
 *   0x80074A30 MenuOtherStep    0x80074B84 func_80074B84
 *   0x800754B4 ConfigOptionStep 0x800755BC ConfigPadStep
 *
 * The top menu's last command opens a short list: the field map (when there
 * is one), the persona data screen, and the config pages. The config pages
 * run as steps of their own.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <persona/common/menuctx.h>
#include <persona/common/bg.h>
#include <persona/adv/personapage.h>

/* The pad layout's byte in the save-game options. */
#define g_pad_layout ((u_char *)0x801F2AC7)

#define OTHER_MAP     0
#define OTHER_PERSONA 1
#define OTHER_CONFIG  2

extern short   g_menu_subsel;
extern short   g_menu_sel;
extern u_char  g_menu_blink;
extern int     g_state_next;
extern int     g_pad_pressed[];
extern u_short D_80100066;

extern void DrawStatusHud(void);
extern void PersonaDataScreen(void);
extern void SoundPlaySeq(u_short slot, u_short seq, short vab);
extern void PadDrawLayout(u_char layout);
extern void PadLoadBindings(u_char config);
extern void PadSetPageButtons(u_char config);
extern void DrawPlaceLabel(short in_battle);
extern void func_8008FC78(u_char n);
extern void ConfigStepTactics(void);
extern void ConfigStepChoice(void);
extern void ConfigCloseChoice(void);
extern void func_800768F0(void);
extern void func_80077F8C(int a, int b);
extern void func_8007A62C(int a, int b);
extern void func_800782A4(int a, int b);
extern void func_80095208(void);
extern void func_80075F14(void);
extern void func_8007584C(void);

void ConfigMenuOpen(void);
void MenuOtherStep(void);
void func_80074B84(void);
void ConfigOptionStep(void);
void ConfigPadStep(void);

void ConfigMenuStep(void)
{
    switch (g_menu_subsel) {
    case 0:
        ConfigMenuOpen();
        g_menu_subsel++;
        break;
    case 1:
        MenuOtherStep();
        break;
    case 2:
        func_80074B84();
        break;
    case 3:
        ConfigOptionStep();
        break;
    case 4:
        ConfigPadStep();
        break;
    case 5:
        func_8007584C();
        break;
    case 6:
        ConfigStepTactics();
        break;
    case 7:
        ConfigStepChoice();
        break;
    case 8:
        ConfigCloseChoice();
        break;
    }
}

void ConfigMenuOpen(void)
{
    func_800768F0();
    SlotSetAnim(0x2D, 0, 0, 0, 0x30, 0x18, 0, 0);
    g_bg_layer_otz[2] = 0x40;
    g_bg_map0.ncellh = 0x40;
    g_bg_map1.ncellh = 0x40;
    g_bg_map0.ncellw = MAP_W;
    g_bg_map1.cellw = 8;
    g_bg_map1.cellh = 12;
    g_bg_layers[2].attribute = 0x8000000;
    g_bg_map1.ncellw = MAP_W;
    g_bg_map2.cellw = 8;
    g_bg_map2.cellh = 12;
    g_bg_map2.ncellw = MAP_W;
    g_bg_map2.ncellh = 0x20;
    func_80077F8C(2, 3);
    func_8007A62C(2, 0x10);
}

void MenuOtherStep(void)
{
    DrawStatusHud();
    if (MenuStepCursor(&g_menu->unk100)) {
        func_800782A4(2, 0x10);
    }
    if (InputCheckAcceptA(2)) {
        switch (g_menu->unk100.cur) {
        case OTHER_MAP:
            if (g_state_next != 2 &&
                (g_state_next != 3 || D_80100066 != 0xFFFF)) {
                func_80095208();
            }
            g_menu_subsel = 0;
            break;
        case OTHER_PERSONA:
            PersonaDataScreen();
            g_menu_subsel = 0;
            break;
        case OTHER_CONFIG:
            func_80075F14();
            g_menu_subsel++;
            break;
        }
    } else if (InputCheckAcceptB(2) || g_menu_allow_hold) {
        g_menu_blink = 0xFF;
        g_menu_sel = 0;
        g_menu_subsel = 0;
    }
}

INCLUDE_ASM("adv/nonmatchings/ui/configmenu", func_80074B84);

/* The option grid, a frame: two columns of on/off pairs. */
void ConfigOptionStep(void)
{
    if (MenuStepCursor(&g_menu->list[0]) || MenuStepCursor(&g_menu->list[1])) {
        SlotSetPos(1, 0x42, g_menu->list[1].cur * 88 + 0x50,
                   g_menu->list[0].cur * 12 + 0x3C);
        func_8008FC78(g_menu->list[1].cur + g_menu->list[0].cur * 2);
    }
    if (InputCheckAcceptB(1) || g_menu_allow_hold) {
        func_80075F14();
        g_menu_subsel--;
    }
}

/* The pad layouts, a frame: the page and the layout on it. Moving to a layout
   takes it at once. */
void ConfigPadStep(void)
{
    u_char *layout;

    if (MenuStepCursor(&g_menu->list[1])) {
        SlotSetPos(1, 0x42, g_menu->list[1].cur * 64 + 0x68, 0x24);
        PadDrawLayout(g_menu->list[1].cur + g_menu->list[0].cur * 2);
        layout = g_pad_layout;
        *layout = g_menu->list[1].cur;
        PadLoadBindings(*layout);
        PadSetPageButtons(*layout);
    } else if (MenuStepCursor(&g_menu->list[0])) {
        DrawPlaceLabel(g_menu->list[0].cur);
        PadDrawLayout(g_menu->list[1].cur + g_menu->list[0].cur * 2);
    }
    /* The page arrows: one page each way. */
    g_slot_cur = &g_slots[PAGE_MARK_SLOT];
    if (g_menu->list[0].cur == 0) {
        g_slot_cur->attr |= SLOT_ATTR_HIDE;
    } else {
        g_slot_cur->attr &= ~SLOT_ATTR_HIDE;
    }
    g_slot_cur++;
    if (g_menu->list[0].cur != 0) {
        g_slot_cur->attr |= SLOT_ATTR_HIDE;
    } else {
        g_slot_cur->attr &= ~SLOT_ATTR_HIDE;
    }
    if (*(u_char *)g_pad_pressed || g_menu_allow_hold) {
        SoundPlaySeq(0x18, 0, 1);
        func_80075F14();
        g_menu_subsel -= 2;
    }
}
