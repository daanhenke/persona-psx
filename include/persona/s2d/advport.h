/* Persona 1 (JP) - building ADV's and DNG's menu units for S2D.
 *
 * Their sources name a few tables and routines by ADV or DNG address; these
 * point the names at S2D's copies. Included by the s2d wrappers ahead of the
 * source they build.
 */
#ifndef PERSONA_S2D_ADVPORT_H
#define PERSONA_S2D_ADVPORT_H

#define D_800B189C D_8009D2EC
#define D_800B1D08 D_8009D758
#define D_800B1E98 D_8009D8E8
#define D_800B2330 D_8009DD80
#define D_800B92A0 D_8009DFF0
#define D_8005E714 g_adv_scene_arg
#define g_pad_held g_pad_held_s2d

#define D_800B12A8 D_8009CCF8
#define D_800B12B8 D_8009CD08
#define D_800B178C D_8009D1DC
#define D_800B1878 D_8009D2C8
#define D_800B1EB8 D_8009D908
#define D_800B2A3C D_8009E4E8
/* The menu icons (src/common/ui/menuicons.c), by ADV's unnamed names. The
   real names are defined as themselves so dngport.h leaves them alone. */
#define MenuWheelOpen      MenuWheelOpen
#define MenuIconsSet5      MenuIconsSet5
#define MenuIconsSet3      MenuIconsSet3
#define MenuIconsSet2      MenuIconsSet2
#define MenuIconsShade     MenuIconsShade
#define MenuIconsShadeFrom MenuIconsShadeFrom
#define func_80077F8C MenuWheelOpen
#define func_8007A508 MenuIconsSet5
#define func_8007A62C MenuIconsSet3
#define D_8007A738    MenuIconsSet2
#define func_8007A83C MenuIconsShade
#define D_8007A8D8    MenuIconsShadeFrom
#define g_BB998 D_800B8630
#define g_BC5C8 D_800B9538
#define DrawStatusFrames EquipScreenLayout
#define D_800BB9A8    D_800B8654
#define D_800BC224    D_800B91DC
#define D_800B17E0    D_8009D230
#define D_800B17E8    D_8009D238
#define D_800B167C    D_8009D0CC
#define D_800B16B0    D_8009D100
#define D_800B188C    D_8009D2DC
#define D_800B1898    D_8009D2E8
#define D_800B198B    D_8009D3DB
#define D_800B1A98    D_8009D4E8
#define D_800B25E8    D_8009E094
#define D_800B8370    D_8009E5AC
#define D_800BB7F8    D_800B5F44
#define g_BB94C       D_800B85C8
#define D_800BC584    D_800B94F8
#define D_800BC594    D_800B9508
#define D_800BC5A4    D_800B9518

/* DNG's. */
#define D_8009ABFC    D_8009D908
#define D_8009AA2C    D_8009D738
#define D_8009AA4C    D_8009D758
#define D_8009AA8C    D_8009D798
#define D_8009AB58    D_8009D864
#define D_8009ABA8    D_8009D8B4
#define D_8009B074    D_8009DD80
#define D_8009B0B4    D_8009DDC0
#define D_8009B0CC    D_8009DDD8
#define D_8009B10C    D_8009DE18

#endif
