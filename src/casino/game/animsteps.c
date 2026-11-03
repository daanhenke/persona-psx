/* Persona 1 (JP) - paired casino object animation steps. */
#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <persona/casino/casino.h>

extern CasinoObj D_8009BB9C, D_8009BD98;
extern CasinoObj D_8009BC14, D_8009BE58;
extern CasinoObj D_8009BC8C, D_8009BF28;
extern void CasinoStartAnim(CasinoObj *o, short frames, short rx, short ry,
                            short rz, long tx, long ty, long tz,
                            long sx, long sy, long sz);

void CasinoAnimStep0(void)
{
    CasinoStartAnim(&D_8009BB9C, 16, 0x800, 0, 0, 0, 0, 0, 0, 0, 0);
    CasinoStartAnim(&D_8009BD98, 16, 0x800, 0, 0, 0, 0, 0, 0, 0, 0);
}

void CasinoAnimStep1(void)
{
    CasinoStartAnim(&D_8009BC14, 16, 0x800, 0, 0, 0, 0, 0, 0, 0, 0);
    CasinoStartAnim(&D_8009BE58, 16, 0x800, 0, 0, 0, 0, 0, 0, 0, 0);
}

void CasinoAnimStep2(void)
{
    CasinoStartAnim(&D_8009BC8C, 16, 0x800, 0, 0, 0, 0, 0, 0, 0, 0);
    CasinoStartAnim(&D_8009BF28, 16, 0x800, 0, 0, 0, 0, 0, 0, 0, 0);
}
