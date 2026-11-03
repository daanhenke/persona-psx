/* Persona 1 (JP) - load the selected TIM and queue its fixed VRAM regions. */
#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libcd.h>
#include <persona/main/cd.h>
extern CdlFILE g_adv_scene_file;
extern u_long *D_800B85BC;
extern int g_cd_busy;
extern void AdvResolveSceneLoc(short, int, void *);
extern void func_80093AE0(void);
extern void VramQueueLoad(u_long *, short, short, short, short);

void func_8009C67C(int scene)
{
    TIM_IMAGE image;
    AdvResolveSceneLoc(3, scene, 0);
    CdReadFileToAddrAsync(&g_adv_scene_file, g_adv_scene_file.size, D_800B85BC);
    while (g_cd_busy != -1) {
        func_80093AE0();
    }
    OpenTIM(D_800B85BC + 2);
    ReadTIM(&image);
    VramQueueLoad(image.caddr, 0x200, 0xF3, 0x100, 1);
    VramQueueLoad(image.paddr, 0x280, 0x180, 0x2C, 0x60);
}
