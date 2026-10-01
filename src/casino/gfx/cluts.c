/* Persona 1 (JP) - CASINO's queue of sprite palettes.
 *   0x80069DFC CasinoQueueCluts
 *   0x80069E70 CasinoQueueClut
 *   0x80069F60 CasinoFlushCluts
 *
 * The palette side of CasinoQueueUV: what flashes or dims a picture
 * without touching its image.
 */
#include <decomp/types.h>
#include <persona/casino/casino.h>

void CasinoQueueClut(short spr, u_short clut);

/* The same palette for a run of sprites. */
void CasinoQueueCluts(short first, short n, int clut)
{
    int i;

    for (i = first; i < first + n; i++) {
        CasinoQueueClut(i, clut);
    }
}

void CasinoQueueClut(short spr, u_short clut)
{
    short no;
    short nb;

    no = CASINO_OTHER(g_casino_cluts.n);
    nb = g_casino_cluts.n[g_casino_buf];
    CASINO_OTHER(g_casino_cluts.spr)[no] = spr;
    g_casino_cluts.spr[g_casino_buf][nb] = spr;
    CASINO_OTHER(g_casino_cluts.clut)[no] = clut;
    g_casino_cluts.clut[g_casino_buf][nb] = clut;
    CASINO_OTHER(g_casino_cluts.n)++;
    g_casino_cluts.n[g_casino_buf]++;
}

void CasinoFlushCluts(void)
{
    int   i;
    short k;

    for (i = 0; i < g_casino_cluts.n[g_casino_buf]; i++) {
        k = g_casino_cluts.spr[g_casino_buf][i];
        g_casino_sprites.prim[g_casino_buf][k].clut = g_casino_cluts.clut[g_casino_buf][i];
    }
    g_casino_cluts.n[g_casino_buf] = 0;
}
