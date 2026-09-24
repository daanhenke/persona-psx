/* Persona 1 (JP) - the persona data screen.  DNG only.
 *   0x80097388 PersonaDataOpen     0x800979A8 PersonaDataScreen
 *   0x80097AF8 ArcanaGridOpen      0x80097F68 ArcanaGridStep
 *   0x800982F8 PersonaDataPick     0x80098520 PersonaDataView
 *   0x80098854 PersonaDataLayout   0x80098C48 PersonaDataDraw
 *
 * The field's copy of ADV's persona data screen (src/adv/ui/personadata.c),
 * built against the field's own message stepper and image queue. Picking a
 * persona reads its portrait off the disc: AdvResolveSceneLoc kind 4 narrows
 * the scene file down to it, nine sectors go to a staging buffer, and the
 * TIM behind the entry's eight-byte header is queued into VRAM.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <libcd.h>
#include <persona/main/cd.h>
#include <persona/common/menuctx.h>
#define SLOT_SETPOS_INT
#define SLOT_TAGGED_INTXY
#include <persona/common/slot.h>

/* The persona ids the list shows, reached by hardcoded address. */
#define g_persona_list ((u_char *)0x800EAE4C)

/* The screen's portraits: kind 4, nine sectors each, read to a staging
   buffer whose first eight bytes are the archive entry's header. */
#define PORTRAIT_KIND    4
#define PORTRAIT_SECTORS 9
#define PORTRAIT_READ    ((u_long *)0x800F4000)
#define PORTRAIT_TIM     ((u_long *)0x800F4008)

/* The sprites. */
#define PICK_CURSOR_SLOT 2
#define ARROW_L_SLOT     0x22
#define ARROW_R_SLOT     0x23
#define HINT_SLOT        0x2C
#define PAGE_TOP_SLOT    0x1E
#define PAGE_BOTTOM_SLOT 0x1F
#define PAGE_MARK_SLOT   32

/* The page scrolls 8 lines a frame to one of two stops. */
#define PAGE_STEP 8
#define PAGE_LOW  0xE0

#define g_slots ((Slot *)0x800DC10C)

/* The two page marks hide at the end they point past. */
#define PAGE_MARKS()                                                              g_slot_cur = &g_slots[PAGE_MARK_SLOT];                                        if (g_cam_y == 0) {                                                               g_slot_cur->attr |= SLOT_ATTR_HIDE;                                       } else {                                                                          g_slot_cur->attr &= ~SLOT_ATTR_HIDE;                                      }                                                                             g_slot_cur++;                                                                 if (g_cam_y == PAGE_LOW) {                                                        g_slot_cur->attr |= SLOT_ATTR_HIDE;                                       } else {                                                                          g_slot_cur->attr &= ~SLOT_ATTR_HIDE;                                      }

extern CdlFILE g_adv_scene_file;
extern u_char  g_persona_list_count;
extern short   g_persona_data_step;
extern u_char  g_pdata_arrow_l_def[];
extern u_char  g_pdata_arrow_r_def[];
extern u_char  g_pdata_cursor_def[];
extern short   g_cam_y;
extern short   g_map_scroll_y;
extern Slot   *g_slot_cur;

/* Declared returning int here, as in menupoll.c: the image tests the
   result unmasked. */
extern int    InputCheckAcceptA(int repeat);
extern int    InputCheckAcceptB(int repeat);
extern void   RunFrame(void);
extern void   AdvResolveSceneLoc(short kind, int index, void *unused);
extern void   TimQueueAt(u_long *tim, short x, short y, short cx, short cy);
extern void   DrawPersonaDataStatBars(int id);
extern void   PersonaDataLayout(void);
/* Defined old-style, as in ADV: the id goes over as the u_char it is. */
extern void   PersonaDataDraw();
extern int    func_80076380(void);
extern void   ArcanaGridOpen(void);
extern void   ArcanaGridStep(void);
extern void   PersonaDataView(void);
void          PersonaDataPick(void);
extern void   SoundPlaySeq(u_short slot, u_short seq, short vab);

extern int     g_pad_held[];
extern int     g_pad_pressed[];
extern u_short g_key_menu_close;
extern int     g_select_toggle;

INCLUDE_ASM("dng/nonmatchings/ui/personadata", PersonaDataOpen);

/* Runs the screen a frame at a time until its step is set to 0xFF: the
   arcana grid, then picking a persona and viewing its page. Select toggles
   g_select_toggle; the close key, once, lets the steps back out. */
void PersonaDataScreen(void)
{
    g_persona_data_step = 0;
    do {
        RunFrame();
        switch (g_persona_data_step) {
        case 0:
            ArcanaGridOpen();
            g_persona_data_step++;
            break;
        case 1:
            ArcanaGridStep();
            break;
        case 2:
            PersonaDataPick();
            break;
        case 3:
            PersonaDataView();
            break;
        }
        if (g_pad_pressed[0] & 0x100) {
            g_select_toggle ^= 1;
        }
        if (!g_menu_allow_hold && (g_key_menu_close & g_pad_pressed[0])) {
            g_menu_allow_hold = 1;
            SoundPlaySeq(0x18, 0, 1);
        }
    } while (g_persona_data_step != 0xFF);
}

INCLUDE_ASM("dng/nonmatchings/ui/personadata", ArcanaGridOpen);

INCLUDE_ASM("dng/nonmatchings/ui/personadata", ArcanaGridStep);

void PersonaDataPick(void)
{
    u_char id;

    MenuStepCursor(&g_menu->list[1]);
    SlotSetPos(PICK_CURSOR_SLOT, 0x42, 0xD8, g_menu->list[1].cur * 12 + 0x30);
    func_80076380();
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
        PersonaDataLayout();
        if (g_persona_list_count >= 2) {
            SlotInitTagged(g_pdata_arrow_l_def, ARROW_L_SLOT, 0x23, 0x56, 0xD8);
            SlotInitTagged(g_pdata_arrow_r_def, ARROW_R_SLOT, 0x23, 0x102, 0xD8);
        }
        SlotClear(HINT_SLOT);
        PersonaDataDraw(id);
        DrawPersonaDataStatBars((short)id);
        g_persona_data_step++;
    } else if (InputCheckAcceptB(1) || g_menu_allow_hold) {
        SlotSetFlicker(1, 1);
        SlotClear(PICK_CURSOR_SLOT);
        g_persona_data_step--;
    }
}

/* The open page: moving the cursor to another persona reads its portrait
   and redraws the page; the page scrolls 8 lines a frame towards the half
   the page cursor picks, and B goes back to the pick. */
void PersonaDataView(void)
{
    int id; /* the persona, then the page's scroll stop */

    if (!MenuStepCursor(&g_menu->page) && MenuStepCursor(&g_menu->list[0])) {
        id = g_persona_list[g_menu->list[0].cur];
        AdvResolveSceneLoc(PORTRAIT_KIND, id, 0);
        CdReadFileToAddrAsync(&g_adv_scene_file, PORTRAIT_SECTORS,
                              PORTRAIT_READ);
        while (g_cd_busy != -1) {
            RunFrame();
        }
        TimQueueAt(PORTRAIT_TIM, 0x140, 0x168, 0, 0x1E6);
        PersonaDataDraw((short)id);
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

    PAGE_MARKS();

    if (InputCheckAcceptB(1) || g_menu_allow_hold) {
        g_menu->list[1].cur = g_menu->list[0].cur;
        ArcanaGridOpen();
        SlotInitTagged(g_pdata_cursor_def, PICK_CURSOR_SLOT, 2, 0xD8,
                       g_menu->list[1].cur * 12 + 0x30);
        SlotSetFlicker(1, 0);
        SlotSetFlicker(PICK_CURSOR_SLOT, 1);
        g_persona_data_step--;
    }
}

INCLUDE_ASM("dng/nonmatchings/ui/personadata", PersonaDataLayout);

INCLUDE_ASM("dng/nonmatchings/ui/personadata", PersonaDataDraw);
