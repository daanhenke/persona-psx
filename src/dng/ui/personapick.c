/* Persona 1 (JP) - picking a persona off the persona data screen's list.
 * DNG only.
 *   0x800982F8 PersonaDataPick
 *
 * The field's copy of ADV's persona data screen (src/adv/ui/personadata.c),
 * built against the field's own message stepper and image queue. Picking a
 * persona reads its portrait off the disc: AdvResolveSceneLoc kind 4 narrows
 * the scene file down to it, nine sectors go to a staging buffer, and the
 * TIM behind the entry's eight-byte header is queued into VRAM.
 */
#include <decomp/types.h>
#include <libcd.h>
#include <persona/main/cd.h>
#include <persona/common/menuctx.h>
#define SLOT_SETPOS_INT
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

extern CdlFILE g_adv_scene_file;
extern u_char  g_persona_list_count;
extern short   g_persona_data_step;
extern u_char  g_pdata_arrow_l_def[];
extern u_char  g_pdata_arrow_r_def[];

/* Declared returning int here, as in menupoll.c: the image tests the
   result unmasked. */
extern int    InputCheckAcceptA(int repeat);
extern int    InputCheckAcceptB(int repeat);
extern void   RunFrame(void);
extern void   AdvResolveSceneLoc(short kind, int index, void *unused);
extern void   TimQueueAt(u_long *tim, short x, short y, short cx, short cy);
extern void   DrawPersonaDataStatBars(short id);
extern void   PersonaDataLayout(void);
/* Defined old-style, as in ADV: the id goes over as the u_char it is. */
extern void   PersonaDataDraw();
extern int    func_80076380(void);

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
        DrawPersonaDataStatBars(id);
        g_persona_data_step++;
    } else if (InputCheckAcceptB(1) || g_menu_allow_hold) {
        SlotSetFlicker(1, 1);
        SlotClear(PICK_CURSOR_SLOT);
        g_persona_data_step--;
    }
}
