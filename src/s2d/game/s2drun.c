/* Persona 1 (JP) - running one map.  S2D.
 *   0x8008B048 S2dRunMap
 *
 * Called once per map by ovl_s2d_entry: the map's palette effect and its
 * models are set up, the LTS file the prefetch started is waited for and its
 * images sent to VRAM, the menu bank opened and the debug print stream set
 * up; then frames run until the map ends. The party's place is written back
 * for the battle or the next map, and the exit the map asked for (in
 * g_s2d_exit) picks how the music is let go and what runs next.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <libsnd.h>
#include <persona/main/cd.h>
#include <persona/main/state.h>
#include <persona/s2d/s2d.h>

#define g_seq_handle ((short *)0x801F537C)
#define LTS_FILE     ((u_long *)0x800D0000)
#define LTS_AT(i)    ((u_long *)(LTS_FILE[i] + (u_long)LTS_FILE))

/* Per map: which palette effect it runs (0xFF for none), and more. */
typedef struct {
    u_short effect;
    u_short pad[11];
} S2dMapInfo;

extern S2dMapInfo g_map_info[];
extern u_char     D_800B1D38[];
extern u_char     D_800A4CFC[];
extern int        D_800B93A0;
extern int        D_800B93A4;
extern int        D_800B93A8;
extern int        D_800B0F18;
extern int        D_800B8650;
extern int        D_800B5F3C;
extern short      D_800B8FD4;
extern short      D_800B8FD8;

extern void TimLoad(u_long *tim);
extern void VramLoad(int x, int y, int w, int h, u_long *p);
extern void S2dBindModels(void);
extern void S2dMarkScript(int kind);
extern void S2dResumeScript(void);
extern void S2dSeekAdvCmd(void);
extern void SoundOpenMenuBank(void);
extern void SoundStopBgm(void);
extern void SoundArmMark(void);
extern void S2dStore266A(void);
extern void S2dStore266B(void);
extern void S2dStoreHeading(void);
extern void S2dStoreScript(void);
extern void func_80096904(void);
extern void func_80097508(u_char *p);
extern void func_80097964(u_char *p);
extern void func_80097BE8(u_char *p);
extern void func_80098BFC(void);
extern void func_8008EC44(void);
extern void func_8008F3C0(void);
extern void func_80091320(void);
extern void func_80089804(void);
extern void func_8008F4A4(void);
extern void func_80091D24(void);
extern void func_8009224C(void);
extern void func_80092A08(int n);
extern void func_8009182C(void);

#ifdef NON_MATCHING
void S2dRunMap(void)
{
    int i;

    func_80096904();
    i = g_map_info[g_map_id].effect;
    switch (i) {
    case 0:
        func_80097508(D_800B1D38);
        break;
    case 1:
        func_80097964(D_800B1D38);
        break;
    }
    if (g_map_info[g_map_id].effect != 0xFF) {
        func_80097BE8(D_800B1D38);
    }
    func_80098BFC();
    S2dBindModels();
    func_8008EC44();
    S2dMarkScript(g_btl_map_id);
    S2dResumeScript();
    D_800B93A0 = 0x1999;
    D_800B93A4 = 0x1000;
    func_8008F3C0();
    func_80091320();

    while (g_cd_busy != -1) {
    }
    i = (int)LTS_AT(0);
    VramLoad(0, 0x100, 0xC0, 0x100, (u_long *)i);
    TimLoad(LTS_AT(1));
    TimLoad(LTS_AT(2));
    DrawSync(0);
    S2dSeekAdvCmd();
    SoundOpenMenuBank();
    FntLoad(0xC0, 0x100);
    DrawSync(0);
    D_800B0F18 = FntOpen(-0xF0, -0x68, 0x200, 0xF0, 0, 0x100);
    SetDumpFnt(D_800B0F18);
    DrawSync(0);
    D_800B8650 = VSync(-1);
    S2dResumeScript();
    D_800B93A0 = 0x1999;
    D_800B93A4 = 0x1000;
    func_80089804();

    D_800B5F3C = 1;
    do {
        func_8008F3C0();
        func_8008F4A4();
    } while (D_800B5F3C != 0);

    D_800B93A0 = 0x1999;
    D_800B93A4 = 0x1000;
    D_800B93A8 = 0x1000;
    S2dStore266A();
    g_btl_pos_x = D_800B8FD4;
    g_btl_pos_y = 0xC7 - D_800B8FD8;
    g_btl_facing = g_s2d_facing;
    if (g_s2d_exit == 1) {
        D_800A4CFC[2] = 0;
    }
    S2dStore266B();
    S2dStoreHeading();
    S2dStoreScript();

    switch (g_s2d_exit) {
    case 0:
        SsSeqSetDecrescendo(g_seq_handle[0], 0x7E, 0x78);
        SoundStopBgm();
        func_8009182C();
        break;
    case 1:
        SsSeqSetDecrescendo(g_seq_handle[0], 0x7E, 0x78);
        SoundStopBgm();
        func_80091D24();
        SsSeqStop(g_seq_handle[0]);
        break;
    case 2:
        for (i = 1; i < 20; i++) {
            if (g_seq_handle[i] != -1) {
                SsSeqStop(g_seq_handle[i]);
            }
        }
        SsSetNck(g_seq_handle[1]);
        SsSetNck(g_seq_handle[2]);
        func_8009224C();
        break;
    case 3:
        func_80092A08(1);
        SoundStopBgm();
        SoundArmMark();
        break;
    case 4:
        SsSeqSetDecrescendo(g_seq_handle[0], 0x7E, 0x78);
        SoundStopBgm();
        func_80092A08(0);
        break;
    }
}
#else
/* 99.7%: the image reads the map id for the effect test into a1 and
   leaves the case-0 jump's slot empty. */
INCLUDE_ASM("s2d/nonmatchings/game/s2drun", S2dRunMap);
#endif
