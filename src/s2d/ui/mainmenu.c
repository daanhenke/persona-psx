/* Persona 1 (JP) - opening the main menu.  S2D.
 *   0x80067D7C MainMenuOpen  0x80067FE8 MainMenu (asm)
 *
 * DNG's MainMenuOpen (src/dng/ui/mainmenu.c) over S2D's addresses: the
 * menu's archive is the one the preload left at 0x800E0000, and the work
 * area sits 0x20000 above DNG's. The screen is cleared and the graphics
 * system set up again, the archive's eight images go to VRAM, and the
 * double ordering table is rebuilt before the menu takes over.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <persona/main/state.h>
#include <persona/common/imageanim.h>
#include <persona/adv/ot.h>

#define PACK_AT ((u_long *)0x800E0000)

extern void VramClearRect(int x, int y, int w, int h);
extern void TimQueueAt(u_long *tim, short x, short y, short cx, short cy);
extern void BgReset(void);

void MainMenu(void);

void MainMenuOpen(void)
{
    u_long *pack = PACK_AT;

    VramClearRect(0, 0, 0x140, 0x1DF);
    SetDispMask(0);
    VSync(0);
    DrawSync(0);
    if (g_state_next == 3) {
        GsInitGraph2(0x140, 0xF0, 0x100, 0, 0);
    } else {
        GsInitGraph(0x140, 0xF0, 0x100, 0, 0);
    }
    GsDefDispBuff(0, 0, 0, 0xF0);

    g_image_queue_count = 0;
    TimQueueAt((u_long *)((u_char *)PACK_AT + pack[1]), 0x380, 0x1C8, 0x100, 0x1F8);
    TimQueueAt((u_long *)((u_char *)PACK_AT + pack[0]), 0x380, 0x100, 0x3C0, 0x1A0);
    TimQueueAt((u_long *)((u_char *)PACK_AT + pack[2]), 0x300, 0x100, 0x3C0, 0x1B0);
    TimQueueAt((u_long *)((u_char *)PACK_AT + pack[3]), 0x200, 0x100, 0, 0x1F8);
    TimQueueAt((u_long *)((u_char *)PACK_AT + pack[4]), 0x280, 0x100, 0, 0x1F0);
    TimQueueAt((u_long *)((u_char *)PACK_AT + pack[5]), 0x300, 0x140, 0x3D0, 0x180);
    TimQueueAt((u_long *)((u_char *)PACK_AT + pack[6]), 0x300, 0x1A0, 0x3E0, 0x180);
    TimQueueAt((u_long *)((u_char *)PACK_AT + pack[7]), 0x300, 0x130, 0x3F0, 0x1B0);
    BgReset();
    FlushImageUploads();
    DrawSync(0);

    g_ot[0].length = g_ot[1].length = 11;
    g_ot[0].org = (GsOT_TAG *)0x800F6000;
    g_ot[1].org = (GsOT_TAG *)0x800F9000;
    g_ot_index = GsGetActiveBuff();
    GsSetWorkBase((PACKET *)(0x800E0000 + g_ot_index * 0xB000));
    GsClearOt(0, 0, &g_ot[g_ot_index]);
    MainMenu();
}

INCLUDE_ASM("s2d/nonmatchings/ui/mainmenu", MainMenu);
