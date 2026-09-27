/* Persona 1 (JP) - building DNG's menu units for ADV.
 *
 * DNG's sources name a few tables and routines by DNG address; these point
 * the names at ADV's copies (ADV's menu data sits 0x172BC above DNG's).
 * Included by the adv wrappers ahead of the source they build.
 */
#ifndef PERSONA_ADV_DNGPORT_H
#define PERSONA_ADV_DNGPORT_H

#define D_8009AA4C    D_800B1D08
#define D_8009ABFC    D_800B1EB8
#define D_8009B074    D_800B2330
#define func_80085AF4 func_80076F78
#define func_80086614 func_80077A98

#endif
