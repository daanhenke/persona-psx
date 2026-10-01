/* Persona 1 (JP) - CASINO's colour fades.
 *   0x8006BB18 CasinoFade
 *   0x8006BB84 CasinoAddFade
 *   0x8006BFF0 CasinoStepFades
 *
 * A run of sprites' tint is moved to a colour over a number of frames: the
 * step per channel is worked out from the first sprite's tint now, and each
 * frame adds it to every sprite of the run.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <libgte.h>
#include <libgpu.h>
#include <persona/casino/casino.h>

void CasinoAddFade(CasinoFadeArg a);

#ifdef NON_MATCHING
void CasinoFade(short first, short count, u_char r, u_char g, u_char b, short frames)
{
    CasinoFadeArg a;

    a.first = first;
    a.count = count;
    a.c.r = r;
    a.c.g = g;
    a.c.b = b;
    a.frames = frames;
    CasinoAddFade(a);
}
#else
/* 65.70%: the same instructions in another order - the image keeps `sw ra`
   and the request's stores ahead of the parameter loads, as in
   CasinoStartAnim. */
INCLUDE_ASM("casino/nonmatchings/gfx/fade", CasinoFade);
#endif

void CasinoAddFade(CasinoFadeArg a)
{
    CasinoRGB c;
    u_char    nb;
    u_char    no;

    nb = g_casino_fades.n[g_casino_buf];
    no = CASINO_OTHER(g_casino_fades.n);
    c = a.c;
    g_casino_fades.first[g_casino_buf][nb] = CASINO_OTHER(g_casino_fades.first)[no] = a.first;
    g_casino_fades.count[g_casino_buf][nb] = CASINO_OTHER(g_casino_fades.count)[no] = a.count;
    g_casino_fades.left[g_casino_buf][nb] = CASINO_OTHER(g_casino_fades.left)[no] = a.frames - 1;
    g_casino_fades.d[g_casino_buf][nb].r = (c.r - g_casino_sprites.prim[g_casino_buf][a.first].r0) / a.frames;
    g_casino_fades.d[g_casino_buf][nb].g = (c.g - g_casino_sprites.prim[g_casino_buf][a.first].g0) / a.frames;
    g_casino_fades.d[g_casino_buf][nb].b = (c.b - g_casino_sprites.prim[g_casino_buf][a.first].b0) / a.frames;
    CASINO_OTHER(g_casino_fades.d)[no].r = (c.r - CASINO_OTHER(g_casino_sprites.prim)[a.first].r0) / a.frames;
    CASINO_OTHER(g_casino_fades.d)[no].g = (c.g - CASINO_OTHER(g_casino_sprites.prim)[a.first].g0) / a.frames;
    CASINO_OTHER(g_casino_fades.d)[no].b = (c.b - CASINO_OTHER(g_casino_sprites.prim)[a.first].b0) / a.frames;
    g_casino_fades.n[g_casino_buf]++;
    CASINO_OTHER(g_casino_fades.n)++;
}

void CasinoStepFades(void)
{
    short first;
    short count;
    int   i;
    int   j;
    int   k;

    for (i = 0; i < g_casino_fades.n[g_casino_buf]; i++) {
        first = g_casino_fades.first[g_casino_buf][i];
        count = g_casino_fades.count[g_casino_buf][i];
        for (j = 0; j < count; j++) {
            g_casino_sprites.prim[g_casino_buf][j + first].r0 += g_casino_fades.d[g_casino_buf][i].r;
            g_casino_sprites.prim[g_casino_buf][j + first].g0 += g_casino_fades.d[g_casino_buf][i].g;
            g_casino_sprites.prim[g_casino_buf][j + first].b0 += g_casino_fades.d[g_casino_buf][i].b;
        }
        if (--g_casino_fades.left[g_casino_buf][i] == -1) {
            for (k = i; k < g_casino_fades.n[g_casino_buf]; k++) {
                g_casino_fades.first[g_casino_buf][k] = g_casino_fades.first[g_casino_buf][k + 1];
                g_casino_fades.count[g_casino_buf][k] = g_casino_fades.count[g_casino_buf][k + 1];
                g_casino_fades.d[g_casino_buf][k] = g_casino_fades.d[g_casino_buf][k + 1];
                g_casino_fades.left[g_casino_buf][k] = g_casino_fades.left[g_casino_buf][k + 1];
            }
            g_casino_fades.n[g_casino_buf]--;
            i--;
        }
    }
}
