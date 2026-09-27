/* dng's object for the shared unit in src/common/ui/menuicons.c: the
   routines still in asm, under DNG's names. */
#include <decomp/include_asm.h>

#define MENUICONS_ASM_5           INCLUDE_ASM("dng/nonmatchings/ui/menuicons", func_80086E20)
#define MENUICONS_ASM_6B          INCLUDE_ASM("dng/nonmatchings/ui/menuicons", func_80087714)
#define MENUICONS_ASM_UPDATE      INCLUDE_ASM("dng/nonmatchings/ui/menuicons", UpdateMenuSprites)
#define MENUICONS_ASM_8           INCLUDE_ASM("dng/nonmatchings/ui/menuicons", D_8008861C)
#define MENUICONS_ASM_SHADEFROM   INCLUDE_ASM("dng/nonmatchings/ui/menuicons", MenuIconsShadeFrom)

#include "../../common/ui/menuicons.c"
