/* Persona 1 (JP) - start two casino animation scripts. */
#include <decomp/types.h>
extern char D_8009BD04[], D_8009BFE8[];
extern void CasinoStartAnim();

void func_8007F578(void)
{
    CasinoStartAnim(D_8009BD04, 16, 0x800, 0, 0, 0, 0, 0, 0, 0, 0);
    CasinoStartAnim(D_8009BFE8, 16, 0x800, 0, 0, 0, 0, 0, 0, 0, 0);
}
