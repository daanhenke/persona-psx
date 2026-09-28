/* adv's object for the shared unit in src/common/ui/menuicons.c: DNG's names
   mapped onto ADV's (dngport.h), and the routines still in asm under ADV's
   names. */
#include <persona/adv/dngport.h>
#include <decomp/include_asm.h>

#define MENUICONS_ASM_SHADEFROM   INCLUDE_ASM("adv/nonmatchings/ui/menuicons", D_8007A8D8)

#include "../../common/ui/menuicons.c"
