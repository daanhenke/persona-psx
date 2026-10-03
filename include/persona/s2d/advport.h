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
#define func_80077F8C func_80076F44
#define func_8007A62C func_800795E4
#define g_BB998 D_800B8630
#define g_BC5C8 D_800B9538
#define DrawStatusFrames EquipScreenLayout
#define D_800BB9A8    D_800B8654
#define D_800BC224    D_800B91DC
#define D_800B17E0    D_8009D230
#define D_800B17E8    D_8009D238
#define D_800B167C    D_8009D0CC
#define D_800B16B0    D_8009D100
#define D_800B1898    D_8009D2E8
#define D_800B198B    D_8009D3DB
#define D_800B1A98    D_8009D4E8

/* DNG's. */
#define D_8009ABFC    D_8009D908

#endif
