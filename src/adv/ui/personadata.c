/* Persona 1 (JP) - the persona data screen.  ADV only.
 *   0x80097AE4 PersonaDataPick   0x80097D20 PersonaDataView
 *
 * Picking a persona off the list opens its data page: the persona's portrait
 * is read off the disc (AdvResolveSceneLoc kind 4 narrows g_adv_scene_file
 * down to it) into a staging buffer and queued into VRAM, and the page's two
 * halves scroll into view one at a time. Moving the cursor on the open page
 * reads the next persona's portrait the same way.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <libcd.h>
#include <persona/main/cd.h>
#include <persona/common/menuctx.h>
#include <persona/common/slot.h>

/* The persona ids the list shows, reached by hardcoded address. */
#define g_persona_list ((u_char *)0x800EAE4C)
#define g_slots        ((Slot *)0x800DC10C)

/* Where a portrait is read to: nine sectors, the archive entry's eight-byte
   header and then the TIM. */
#define PORTRAIT_KIND    4
#define PORTRAIT_SECTORS 9
#define PORTRAIT_READ    ((u_long *)0x800F4000)
#define PORTRAIT_TIM     ((u_long *)0x800F4008)

/* The sprites. */
#define PICK_CURSOR_SLOT 2
#define ARROW_L_SLOT     0x22
#define ARROW_R_SLOT     0x23
#define PAGE_TOP_SLOT    0x1E
#define PAGE_BOTTOM_SLOT 0x1F
#define PAGE_MARK_SLOT   32
#define HINT_SLOT        0x2C

/* The page scrolls 8 lines a frame to one of two stops. */
#define PAGE_STEP 8
#define PAGE_LOW  0xE0

extern u_char InputCheckAcceptA(u_char repeat);
extern u_char InputCheckAcceptB(u_char repeat);
extern int    MsgStep(void);
extern void   RunFrame(void);
extern void   TimQueueAt(u_long *tim, short x, short y, short cx, short cy);
extern void   AdvResolveSceneLoc(short kind, int index, void *unused);
extern void   DrawPersonaDataStatBars(short id);
extern void   SoundPlaySeq(u_short slot, u_short seq, short vab);

extern CdlFILE g_adv_scene_file;
extern u_char  g_persona_list_count;
extern short   D_800BC040;
extern short   g_cam_y;
extern short   g_map_scroll_y;
extern Slot   *g_slot_cur;
extern int     g_pad_pressed[];
extern int     g_BB998;
extern u_short D_801F2A94;

extern u_char D_800B10BC[];
extern u_char D_800B10E4[];
extern u_char D_800B1230[];

void func_800972AC(void);
void func_8009772C(void);
void PersonaDataPick(void);
void PersonaDataView(void);
void func_80098074(void);
void func_80098480(short id);

INCLUDE_ASM("adv/nonmatchings/ui/personadata", func_80096B30);

void PersonaDataScreen(void)
{
    D_800BC040 = 0;
    do {
        RunFrame();
        switch (D_800BC040) {
        case 0:
            func_800972AC();
            D_800BC040++;
            break;
        case 1:
            func_8009772C();
            break;
        case 2:
            PersonaDataPick();
            break;
        case 3:
            PersonaDataView();
            break;
        }
        if (g_pad_pressed[0] & 0x100) {
            g_BB998 ^= 1;
        }
        if (!g_menu_allow_hold && (D_801F2A94 & g_pad_pressed[0])) {
            g_menu_allow_hold = 1;
            SoundPlaySeq(0x18, 0, 1);
        }
    } while (D_800BC040 != 0xFF);
}

INCLUDE_ASM("adv/nonmatchings/ui/personadata", func_800972AC);

INCLUDE_ASM("adv/nonmatchings/ui/personadata", func_8009772C);

void PersonaDataPick(void)
{
    u_char id;

    MenuStepCursor(&g_menu->list[1]);
    SlotSetPos(PICK_CURSOR_SLOT, 0x42, 0xD8, g_menu->list[1].cur * 12 + 0x30);
    MsgStep();
    if (InputCheckAcceptA(1)) {
        MenuListInit(&g_menu->list[0], g_menu->list[1].cur, 0,
                     g_persona_list_count - 1, 0x1A);
        id = g_persona_list[g_menu->list[1].cur];
        AdvResolveSceneLoc(PORTRAIT_KIND, id, 0);
        CdReadFileToAddrAsync(&g_adv_scene_file, PORTRAIT_SECTORS,
                              PORTRAIT_READ);
        while (g_cd_busy != -1) {
            RunFrame();
        }
        TimQueueAt(PORTRAIT_TIM, 0x140, 0x168, 0, 0x1E6);
        MenuListInit(&g_menu->page, 0, 0, 1, 0x14);
        func_80098074();
        if (g_persona_list_count >= 2) {
            SlotInitTagged(D_800B10BC, ARROW_L_SLOT, 0x23, 0x56, 0xD8);
            SlotInitTagged(D_800B10E4, ARROW_R_SLOT, 0x23, 0x102, 0xD8);
        }
        SlotClear(HINT_SLOT);
        func_80098480(id);
        DrawPersonaDataStatBars(id);
        D_800BC040++;
    } else if (InputCheckAcceptB(1) || g_menu_allow_hold) {
        SlotSetFlicker(1, 1);
        SlotClear(PICK_CURSOR_SLOT);
        D_800BC040--;
    }
}

void PersonaDataView(void)
{
    int id;   /* the persona, then the page's scroll stop */

    if (!MenuStepCursor(&g_menu->page) && MenuStepCursor(&g_menu->list[0])) {
        id = g_persona_list[g_menu->list[0].cur];
        AdvResolveSceneLoc(PORTRAIT_KIND, id, 0);
        CdReadFileToAddrAsync(&g_adv_scene_file, PORTRAIT_SECTORS,
                              PORTRAIT_READ);
        while (g_cd_busy != -1) {
            RunFrame();
        }
        TimQueueAt(PORTRAIT_TIM, 0x140, 0x168, 0, 0x1E6);
        func_80098480(id);
        DrawPersonaDataStatBars(id);
    }

    switch (g_menu->page.cur) {
    case 0:
        id = 0;
        break;
    case 1:
        id = PAGE_LOW;
        break;
    }
    if (id < g_cam_y) {
        g_cam_y -= PAGE_STEP;
    }
    if (g_cam_y < id) {
        g_cam_y += PAGE_STEP;
    }
    if (id < g_map_scroll_y) {
        g_map_scroll_y -= PAGE_STEP;
    }
    if (g_map_scroll_y < id) {
        g_map_scroll_y += PAGE_STEP;
    }
    SlotSetPos(PAGE_TOP_SLOT, 0x50, 0x44, 0x40 - g_cam_y);
    SlotSetPos(PAGE_BOTTOM_SLOT, 0x50, 0x48, 0x120 - g_cam_y);

    /* The two page marks hide at the end they point past. */
    g_slot_cur = &g_slots[PAGE_MARK_SLOT];
    if (g_cam_y == 0) {
        g_slot_cur->attr |= SLOT_ATTR_HIDE;
    } else {
        g_slot_cur->attr &= ~SLOT_ATTR_HIDE;
    }
    g_slot_cur++;
    if (g_cam_y == PAGE_LOW) {
        g_slot_cur->attr |= SLOT_ATTR_HIDE;
    } else {
        g_slot_cur->attr &= ~SLOT_ATTR_HIDE;
    }

    if (InputCheckAcceptB(1) || g_menu_allow_hold) {
        g_menu->list[1].cur = g_menu->list[0].cur;
        func_800972AC();
        SlotInitTagged(D_800B1230, PICK_CURSOR_SLOT, 2, 0xD8,
                       g_menu->list[1].cur * 12 + 0x30);
        SlotSetFlicker(1, 0);
        SlotSetFlicker(PICK_CURSOR_SLOT, 1);
        D_800BC040--;
    }
}

INCLUDE_ASM("adv/nonmatchings/ui/personadata", func_80098074);

INCLUDE_ASM("adv/nonmatchings/ui/personadata", func_80098480);
