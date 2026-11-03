/* Persona 1 (JP) - move three layout pieces apart for a card display. */
#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <persona/casino/casino.h>
extern void CasinoTween();

void func_8007EE78(CasinoFrame *pieces, short distance, short duration)
{
    int outward = distance * 8;
    int inward = -outward;
    CasinoTween(pieces[0].l, inward, 0, 0, 0, duration);
    CasinoTween(pieces[1].l, outward, 0, 0, 0, duration);
    CasinoTween(pieces[2].l, inward, 0, distance * 16, 0, duration);
}
