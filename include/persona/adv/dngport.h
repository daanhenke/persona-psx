/* Persona 1 (JP) - building DNG's menu units for ADV.
 *
 * DNG's sources name a few tables and routines by DNG address; these point
 * the names at ADV's copies (ADV's menu data sits 0x172BC above DNG's).
 * Included by the adv wrappers ahead of the source they build. S2D builds
 * some of those ADV units and maps the same names first, so each is guarded.
 */
#ifndef PERSONA_ADV_DNGPORT_H
#define PERSONA_ADV_DNGPORT_H

#ifndef D_8009AA4C
#define D_8009AA4C    D_800B1D08
#endif
#ifndef D_8009ABFC
#define D_8009ABFC    D_800B1EB8
#endif
#ifndef D_8009B074
#define D_8009B074    D_800B2330
#endif
#ifndef func_80085AF4
#define func_80085AF4 func_80076F78
#endif
#ifndef func_80086614
#define func_80086614 func_80077A98
#endif

/* The equipment screen's (src/common/ui/equipsteps.c). */
#ifndef D_8009FE20
#define D_8009FE20          D_800BB9A8
#endif
#ifndef D_800A04D4
#define D_800A04D4          D_800BC224
#endif
#ifndef CharPreviewDraw
#define CharPreviewDraw     EquipDrawCompare
#endif
#ifndef EquipShowListCursor
#define EquipShowListCursor EquipPlaceCursor
#endif
#ifndef EquipDrawListRow
#define EquipDrawListRow    DrawItemRow
#endif
#ifndef EquipScreenLayout
#define EquipScreenLayout   DrawStatusFrames
#endif

/* The map screen's player marker (src/dng/ui/mapstep.c). */
#ifndef D_8009FDFC
#define D_8009FDFC D_800BB990
#endif
#ifndef D_8009FE08
#define D_8009FE08 D_800BB99C
#endif
#ifndef D_8009FE0C
#define D_8009FE0C D_800BB9A0
#endif

#endif
