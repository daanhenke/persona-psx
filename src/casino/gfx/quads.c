/* Persona 1 (JP) - CASINO's queue of sprite corners.
 *   0x80069660 CasinoQueueQuad
 *   0x80069894 CasinoFlushQuads
 *
 * A sprite is moved by queueing its new corners; each buffer's polygons
 * take them when that buffer is next drawn, so the queue holds the entry
 * once per buffer.
 */
#include <decomp/types.h>
#include <persona/casino/casino.h>

void CasinoQueueQuad(short x0, short y0, short x1, short y1, short x2, short y2, short x3, short y3, int spr)
{
    int no;
    int nb;

    no = CASINO_OTHER(g_casino_quads.n);
    nb = g_casino_quads.n[g_casino_buf];
    CASINO_OTHER(g_casino_quads.quad)[no].x0 = x0;
    CASINO_OTHER(g_casino_quads.quad)[no].y0 = y0;
    CASINO_OTHER(g_casino_quads.quad)[no].x1 = x1;
    CASINO_OTHER(g_casino_quads.quad)[no].y1 = y1;
    CASINO_OTHER(g_casino_quads.quad)[no].x2 = x2;
    CASINO_OTHER(g_casino_quads.quad)[no].y2 = y2;
    CASINO_OTHER(g_casino_quads.quad)[no].x3 = x3;
    CASINO_OTHER(g_casino_quads.quad)[no].y3 = y3;
    g_casino_quads.quad[g_casino_buf][nb].x0 = x0;
    g_casino_quads.quad[g_casino_buf][nb].y0 = y0;
    g_casino_quads.quad[g_casino_buf][nb].x1 = x1;
    g_casino_quads.quad[g_casino_buf][nb].y1 = y1;
    g_casino_quads.quad[g_casino_buf][nb].x2 = x2;
    g_casino_quads.quad[g_casino_buf][nb].y2 = y2;
    g_casino_quads.quad[g_casino_buf][nb].x3 = x3;
    g_casino_quads.quad[g_casino_buf][nb].y3 = y3;
    CASINO_OTHER(g_casino_quads.spr)[no] = spr;
    g_casino_quads.spr[g_casino_buf][nb] = spr;
    CASINO_OTHER(g_casino_quads.n)++;
    g_casino_quads.n[g_casino_buf]++;
}

void CasinoFlushQuads(void)
{
    int   i;
    short k;

    for (i = 0; i < g_casino_quads.n[g_casino_buf]; i++) {
        k = g_casino_quads.spr[g_casino_buf][i];
        g_casino_sprites.prim[g_casino_buf][k].x0 = g_casino_quads.quad[g_casino_buf][i].x0;
        g_casino_sprites.prim[g_casino_buf][k].y0 = g_casino_quads.quad[g_casino_buf][i].y0;
        g_casino_sprites.prim[g_casino_buf][k].x1 = g_casino_quads.quad[g_casino_buf][i].x1;
        g_casino_sprites.prim[g_casino_buf][k].y1 = g_casino_quads.quad[g_casino_buf][i].y1;
        g_casino_sprites.prim[g_casino_buf][k].x2 = g_casino_quads.quad[g_casino_buf][i].x2;
        g_casino_sprites.prim[g_casino_buf][k].y2 = g_casino_quads.quad[g_casino_buf][i].y2;
        g_casino_sprites.prim[g_casino_buf][k].x3 = g_casino_quads.quad[g_casino_buf][i].x3;
        g_casino_sprites.prim[g_casino_buf][k].y3 = g_casino_quads.quad[g_casino_buf][i].y3;
    }
    g_casino_quads.n[g_casino_buf] = 0;
}
