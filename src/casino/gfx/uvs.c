/* Persona 1 (JP) - CASINO's queue of sprite textures.
 *   0x80069A10 CasinoQueueUV
 *   0x80069BD4 CasinoFlushUVs
 *
 * The texture side of CasinoQueueQuad: a sprite's image rectangle, page
 * and palette, queued for both buffers and applied to each buffer's
 * polygon as it is next drawn.
 */
#include <decomp/types.h>
#include <persona/casino/casino.h>

void CasinoQueueUV(short u, short v, short w, short h, u_short tpage, u_short clut, int spr)
{
    int no;
    int nb;

    no = CASINO_OTHER(g_casino_uvs.n);
    nb = g_casino_uvs.n[g_casino_buf];
    CASINO_OTHER(g_casino_uvs.uv)[no].u = u;
    CASINO_OTHER(g_casino_uvs.uv)[no].v = v;
    CASINO_OTHER(g_casino_uvs.uv)[no].w = w;
    CASINO_OTHER(g_casino_uvs.uv)[no].h = h;
    g_casino_uvs.uv[g_casino_buf][nb].u = u;
    g_casino_uvs.uv[g_casino_buf][nb].v = v;
    g_casino_uvs.uv[g_casino_buf][nb].w = w;
    g_casino_uvs.uv[g_casino_buf][nb].h = h;
    CASINO_OTHER(g_casino_uvs.tpage)[no] = tpage;
    g_casino_uvs.tpage[g_casino_buf][nb] = tpage;
    CASINO_OTHER(g_casino_uvs.clut)[no] = clut;
    g_casino_uvs.clut[g_casino_buf][nb] = clut;
    CASINO_OTHER(g_casino_uvs.spr)[no] = spr;
    g_casino_uvs.spr[g_casino_buf][nb] = spr;
    CASINO_OTHER(g_casino_uvs.n)++;
    g_casino_uvs.n[g_casino_buf]++;
}

void CasinoFlushUVs(void)
{
    int       i;
    short     k;
    CasinoUV *uv;

    for (i = 0; i < g_casino_uvs.n[g_casino_buf]; i++) {
        k = g_casino_uvs.spr[g_casino_buf][i];
        uv = &g_casino_uvs.uv[g_casino_buf][i];
        g_casino_sprites.prim[g_casino_buf][k].u0 = uv->u;
        g_casino_sprites.prim[g_casino_buf][k].v0 = uv->v;
        g_casino_sprites.prim[g_casino_buf][k].u1 = uv->u + uv->w;
        g_casino_sprites.prim[g_casino_buf][k].v1 = uv->v;
        g_casino_sprites.prim[g_casino_buf][k].u2 = uv->u;
        g_casino_sprites.prim[g_casino_buf][k].v2 = uv->v + uv->h;
        g_casino_sprites.prim[g_casino_buf][k].u3 = uv->u + uv->w;
        g_casino_sprites.prim[g_casino_buf][k].v3 = uv->v + uv->h;
        g_casino_sprites.prim[g_casino_buf][k].tpage = g_casino_uvs.tpage[g_casino_buf][i];
        g_casino_sprites.prim[g_casino_buf][k].clut = g_casino_uvs.clut[g_casino_buf][i];
    }
    g_casino_uvs.n[g_casino_buf] = 0;
}
