/* Persona 1 (JP) - build reel layouts, objects and their palette queues. */
#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <persona/casino/casino.h>
extern void CasinoBuildLayouts(CasinoLayoutDef *, int);
extern void CasinoBuildFrame(CasinoFrame *);
extern void CasinoBuildObj(CasinoModel *);
extern void CasinoTexObj(CasinoModel *);
extern void CasinoSetXform(CasinoXform *);
extern void CasinoDrawObj(CasinoObj *);
extern void CasinoQueueCluts();
extern short reel_clut_x __asm__("D_800999C0+0x8");
extern short reel_clut_y __asm__("D_800999C0+0xA");
extern CasinoLayoutDef D_800A086C;
extern CasinoLayoutDef D_800A09E0;
extern CasinoLayoutDef D_800A0CEC;
extern CasinoLayoutDef D_800A0EEC;
extern CasinoLayoutDef D_800A0410;
extern CasinoLayoutDef D_800A0528;
extern CasinoLayoutDef D_800A0664;
extern CasinoLayoutDef D_800A078C;
extern CasinoLayoutDef D_800A1128;
extern CasinoLayoutDef D_800A1244;
extern CasinoLayoutDef D_800A1428;
extern CasinoLayoutDef D_800A12B4;
extern CasinoLayoutDef D_800A15C8;
extern CasinoLayoutDef D_800A1620;
extern CasinoLayoutDef D_800A1764;
extern CasinoLayoutDef D_800A17B8;
extern CasinoLayoutDef D_800A1A54;
extern CasinoLayoutDef D_800A20EC;
extern CasinoLayoutDef D_800A24D4;
extern CasinoLayoutDef D_800A10B0;
extern CasinoLayoutDef D_800A2644;
extern CasinoLayoutDef D_800A2734;
extern CasinoLayoutDef D_800A2844;
extern CasinoLayoutDef D_800A2B40;
extern CasinoFrame D_800A0580;
extern CasinoModel D_800A14EC;
extern CasinoModel D_800A157C;
extern CasinoModel D_800A1708;
extern CasinoObj D_800A14DC;
extern CasinoObj D_800A156C;
extern CasinoObj D_800A16F8;
extern CasinoXform *D_800A14E8;
extern CasinoXform *D_800A1578;
extern CasinoXform *D_800A1704;

void func_8008514C(void) {
    CasinoBuildLayouts(&D_800A086C, 0);
    CasinoBuildLayouts(&D_800A09E0, 0);
    CasinoBuildLayouts(&D_800A0CEC, 0);
    CasinoBuildLayouts(&D_800A0EEC, 0);
    CasinoBuildLayouts(&D_800A0410, 0);
    CasinoBuildLayouts(&D_800A0528, 0);
    CasinoBuildFrame(&D_800A0580);
    CasinoBuildLayouts(&D_800A0664, 0);
    CasinoBuildLayouts(&D_800A078C, 0);
    CasinoBuildLayouts(&D_800A1128, 0);
    CasinoBuildLayouts(&D_800A1244, 0);
    CasinoBuildLayouts(&D_800A1428, 0);
    CasinoBuildLayouts(&D_800A12B4, 0);
    CasinoBuildObj(&D_800A14EC);
    CasinoTexObj(&D_800A14EC);
    CasinoSetXform(D_800A14E8);
    CasinoDrawObj(&D_800A14DC);
    CasinoBuildObj(&D_800A157C);
    CasinoTexObj(&D_800A157C);
    CasinoSetXform(D_800A1578);
    CasinoDrawObj(&D_800A156C);
    CasinoBuildLayouts(&D_800A15C8, 0);
    CasinoBuildLayouts(&D_800A1620, 0);
    CasinoBuildObj(&D_800A1708);
    CasinoTexObj(&D_800A1708);
    CasinoSetXform(D_800A1704);
    CasinoDrawObj(&D_800A16F8);
    CasinoBuildLayouts(&D_800A1764, 0);
    CasinoBuildLayouts(&D_800A17B8, 0);
    CasinoBuildLayouts(&D_800A1A54, 0);
    CasinoBuildLayouts(&D_800A20EC, 0);
    CasinoBuildLayouts(&D_800A24D4, 0);
    CasinoBuildLayouts(&D_800A10B0, 0);
    CasinoBuildLayouts(&D_800A2644, 0);
    CasinoBuildLayouts(&D_800A2734, 0);
    CasinoBuildLayouts(&D_800A2844, 0);
    CasinoBuildLayouts(&D_800A2B40, 0);
    CasinoQueueCluts(0x195, 0xE, GetClut(reel_clut_x, reel_clut_y) & 0xFFFF);
    CasinoQueueCluts(0x1A3, 7, GetClut(reel_clut_x, reel_clut_y) & 0xFFFF);
}
