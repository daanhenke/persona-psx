#ifndef PERSONA_MAIN_PSEXE_H
#define PERSONA_MAIN_PSEXE_H

/* Persona 1 (JP) - SLPS_005.00 PS-EXE loader.
 *
 * Defined in src/p1-jp/main/psexe.c and spun on by the sub-EXE launcher in
 * src/p1-jp/main/boot.c, which is why the prototype is shared.
 */
#include <libapi.h>

extern int CdLoadPsExe(const char *name, struct EXEC *exec);

#endif
