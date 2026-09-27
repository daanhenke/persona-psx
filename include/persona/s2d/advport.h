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

/* DNG's. */
#define D_8009ABFC    D_8009D908
#define func_80085AF4 func_80075F30
#define func_80086614 func_80076A50

#endif
