/* Persona 1 (JP) - CASINO's layouts: pictures made of runs of sprites.
 *   0x800684D8 CasinoBuildFrame
 *   0x800686FC CasinoBuildLayouts
 *   0x8006877C CasinoBuildGrid
 *   0x800688B8 CasinoShowLayout
 *   0x80068CB0 CasinoResetLayout
 *   0x80068EE4 CasinoTexLayout
 *   0x80068F98 CasinoQueueRect
 *   0x8006924C CasinoFlushRects
 *
 * A layout's pieces each own a sprite (first + i). Building one fills its
 * cell table, from a grid definition or a four-piece frame; showing it
 * queues every piece's rectangle, texture or both, and zeroes the running
 * offsets tweens add on top.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <libgte.h>
#include <libgpu.h>
#include <persona/casino/casino.h>

extern void CasinoQueueQuad(short x0, short y0, short x1, short y1, short x2, short y2, short x3, short y3, int spr);
extern void CasinoQueueUV(short u, short v, short w, short h, u_short tpage, u_short clut, int spr);

/* A frame of four pieces: two of w0 by h0 and two of w1 by h1, taken from
   one place in a texture page. */
typedef struct {
    short   x;
    short   y;
    short   u;
    short   v;
    short   w0;
    short   h0;
    short   w1;
    short   h1;
    u_short z;
    u_char  on;
    u_char  pad;
    u_short tpage;
    u_short clut;
} CasinoFrameDef;

typedef struct {
    CasinoFrameDef *def;
    CasinoLayout   *l;
} CasinoFrame;

void CasinoBuildGrid(CasinoGridDef *g, CasinoLayout *l);
void CasinoShowLayout(CasinoLayout *l, u_char mode);
void CasinoResetLayout(CasinoLayout *l, u_char mode);
void CasinoQueueRect(short x, short y, short w, short h, short u, short v, short uw, short vh, u_short tpage,
                     u_short clut, short spr);

#ifdef NON_MATCHING
void CasinoBuildFrame(CasinoFrame *d)
{
    volatile short tab[16];
    short   x;
    short   y;
    short   u;
    short   v;
    short   w0;
    short   h0;
    short   w1;
    short   h1;
    u_short z;
    u_char  on;
    u_short tpage;
    u_short clut;
    int     i;

    x = d->def->x;
    y = d->def->y;
    u = d->def->u;
    v = d->def->v;
    w0 = d->def->w0;
    h0 = d->def->h0;
    w1 = d->def->w1;
    h1 = d->def->h1;
    z = d->def->z;
    on = d->def->on;
    tpage = d->def->tpage;
    clut = d->def->clut;
    tab[2] = 0;
    tab[0] = w1;
    tab[1] = w1;
    tab[3] = w1 + w0;
    tab[4] = 0;
    tab[5] = h1 - h0;
    tab[6] = 0;
    tab[7] = 0;
    tab[8] = w0;
    tab[9] = w0;
    tab[10] = w1;
    tab[11] = w1;
    tab[12] = h0;
    tab[13] = h0;
    tab[14] = h1;
    tab[15] = h1;
    for (i = 0; i < 4; i++) {
        d->l->cells[i].x = x + tab[i];
        d->l->cells[i].y = y + tab[i + 4];
        d->l->cells[i].w = tab[i + 8];
        d->l->cells[i].h = tab[i + 12];
        d->l->cells[i].z = z;
        d->l->cells[i].on = on;
        d->l->tex[i].u = u + tab[i];
        d->l->tex[i].v = v + tab[i + 4];
        d->l->tex[i].w = tab[i + 8];
        d->l->tex[i].h = tab[i + 12];
        d->l->tex[i].tpage = tpage;
        d->l->tex[i].clut = clut;
        d->l->cell_idx[i] = i;
        d->l->tex_idx[i] = i;
    }
    CasinoShowLayout(d->l, 0);
}
#else
/* 97.37%: the table is volatile and filled in the image's order, which is
   what keeps its stores in place; what is left is d and the cell offset
   trading registers (a3/t0). */
INCLUDE_ASM("casino/nonmatchings/gfx/layout", CasinoBuildFrame);
#endif

/* A list of grids ends at a definition of -1. */
void CasinoBuildLayouts(CasinoLayoutDef *d, int mode)
{
    while (d->def != (void *)-1) {
        CasinoBuildGrid(d->def, d->l);
        CasinoShowLayout(d->l, mode);
        d++;
    }
}

void CasinoBuildGrid(CasinoGridDef *g, CasinoLayout *l)
{
    CasinoCell *c;
    short      *idx;
    short       cols;
    short       x;
    short       y;
    short       w;
    short       h;
    short       dx;
    short       dy;
    u_short     z;
    u_char      on;
    short       rows;
    int         r;
    int         k;

    c = l->cells;
    rows = g->rows;
    idx = l->cell_idx;
    cols = g->cols;
    x = g->x;
    y = g->y;
    w = g->w;
    h = g->h;
    dx = g->dx;
    dy = g->dy;
    z = g->z;
    on = g->on;
    for (r = 0; r < rows; r++) {
        for (k = 0; k < cols; k++) {
            c->x = x + k * dx + k * w;
            c->y = y + dy * r + h * r;
            c->w = w;
            c->h = h;
            c->z = z;
            c->on = on;
            c++;
            *idx++ = r * cols + k;
        }
    }
}

/* Mode 0 queues each piece whole, 1 only its corners, 2 only its texture. */
void CasinoShowLayout(CasinoLayout *l, u_char mode)
{
    short      *cidx;
    short      *tidx;
    u_short     first;
    u_short     n;
    CasinoCell *cells;
    CasinoTex  *tex;
    u_short     clut;
    int         k;
    int         i;

    cidx = l->cell_idx;
    tidx = l->tex_idx;
    first = l->first;
    n = l->n;
    cells = l->cells;
    tex = l->tex;
    CasinoResetLayout(l, mode);
    switch (mode) {
    case 0:
        for (i = 0; i < n; i++) {
            CasinoQueueRect(cells[*cidx].x, cells[*cidx].y, cells[*cidx].w, cells[*cidx].h, tex[tidx[i]].u, tex[tidx[i]].v, tex[tidx[i]].w,
                            tex[tidx[i]].h, tex[tidx[i]].tpage, tex[tidx[i]].clut, first + i);
            g_casino_sprites.z[first + i] = cells[*cidx].z;
            g_casino_sprites.on[first + i] = cells[*cidx].on;
            cidx++;
        }
        break;
    case 1:
        for (i = 0; i < n; i++) {
            RECT r;

            r.x = cells[*cidx].x;
            r.y = cells[*cidx].y;
            r.w = cells[*cidx].w;
            r.h = cells[*cidx].h;
            CasinoQueueQuad(r.x, r.y, r.x + r.w, r.y, r.x, r.y + r.h, r.x + r.w, r.y + r.h, first + i);
            g_casino_sprites.z[first + i] = cells[*cidx].z;
            g_casino_sprites.on[first + i] = cells[*cidx].on;
            cidx++;
        }
        break;
    case 2:
        for (i = 0; i < n; i++) {
            clut = tex[*tidx].clut;
            k = first + i;
            CasinoQueueUV(tex[*tidx].u, tex[*tidx].v, tex[*tidx].w, tex[*tidx].h, tex[*tidx].tpage, clut, k);
            tidx++;
        }
        break;
    }
}

/* Clears the running offsets of a layout's sprites and takes their depth,
   visibility, page and palette from the layout. Mode 2 never sets k, so it
   rewrites one entry, wherever k happens to point, n times. */
void CasinoResetLayout(CasinoLayout *l, u_char mode)
{
    short n;
    int   i;
    int   k;

    n = l->n;
    switch (mode) {
    case 0:
        for (i = 0; i < n; i++) {
            k = i + l->first;
            g_casino_cells[k].x = 0;
            g_casino_cells[k].y = 0;
            g_casino_cells[k].w = 0;
            g_casino_cells[k].h = 0;
            g_casino_cells[k].z = l->cells[i].z;
            g_casino_cells[k].on = l->cells[i].on;
            g_casino_texs[k].u = 0;
            g_casino_texs[k].v = 0;
            g_casino_texs[k].w = 0;
            g_casino_texs[k].h = 0;
            g_casino_texs[k].tpage = l->tex[i].tpage;
            g_casino_texs[k].clut = l->tex[i].clut;
        }
        break;
    case 1:
        for (i = 0; i < n; i++) {
            k = i + l->first;
            g_casino_cells[k].x = 0;
            g_casino_cells[k].y = 0;
            g_casino_cells[k].w = 0;
            g_casino_cells[k].h = 0;
            g_casino_cells[k].z = l->cells[i].z;
            g_casino_cells[k].on = l->cells[i].on;
        }
        break;
    case 2:
        for (i = 0; i < n; i++) {
            g_casino_texs[k].u = 0;
            g_casino_texs[k].v = 0;
            g_casino_texs[k].w = 0;
            g_casino_texs[k].h = 0;
            g_casino_texs[k].tpage = l->tex[i].tpage;
            g_casino_texs[k].clut = l->tex[i].clut;
        }
        break;
    }
}

void CasinoTexLayout(CasinoLayout *l)
{
    CasinoTex *t;
    short     *idx;
    u_short    clut;
    int        i;

    idx = l->tex_idx;
    for (i = l->first; i < l->first + l->n; i++) {
        t = &l->tex[*idx++];
        clut = t->clut;
        CasinoQueueUV(t->u, t->v, t->w, t->h, t->tpage, clut, i);
    }
}

void CasinoQueueRect(short x, short y, short w, short h, short u, short v, short uw, short vh, u_short tpage,
                     u_short clut, short spr)
{
    int no;
    int nb;

    no = CASINO_OTHER(g_casino_rects.n);
    nb = g_casino_rects.n[g_casino_buf];
    CASINO_OTHER(g_casino_rects.pos)[no].x = x;
    CASINO_OTHER(g_casino_rects.pos)[no].y = y;
    CASINO_OTHER(g_casino_rects.pos)[no].w = w;
    CASINO_OTHER(g_casino_rects.pos)[no].h = h;
    g_casino_rects.pos[g_casino_buf][nb].x = x;
    g_casino_rects.pos[g_casino_buf][nb].y = y;
    g_casino_rects.pos[g_casino_buf][nb].w = w;
    g_casino_rects.pos[g_casino_buf][nb].h = h;
    CASINO_OTHER(g_casino_rects.uv)[no].x = u;
    CASINO_OTHER(g_casino_rects.uv)[no].y = v;
    CASINO_OTHER(g_casino_rects.uv)[no].w = uw;
    CASINO_OTHER(g_casino_rects.uv)[no].h = vh;
    g_casino_rects.uv[g_casino_buf][nb].x = u;
    g_casino_rects.uv[g_casino_buf][nb].y = v;
    g_casino_rects.uv[g_casino_buf][nb].w = uw;
    g_casino_rects.uv[g_casino_buf][nb].h = vh;
    CASINO_OTHER(g_casino_rects.tpage)[no] = tpage;
    g_casino_rects.tpage[g_casino_buf][nb] = tpage;
    CASINO_OTHER(g_casino_rects.clut)[no] = clut;
    g_casino_rects.clut[g_casino_buf][nb] = clut;
    CASINO_OTHER(g_casino_rects.spr)[no] = spr;
    g_casino_rects.spr[g_casino_buf][nb] = spr;
    CASINO_OTHER(g_casino_rects.n)++;
    g_casino_rects.n[g_casino_buf]++;
}

void CasinoFlushRects(void)
{
    RECT  p;
    RECT  t;
    int   i;
    short k;

    for (i = 0; i < g_casino_rects.n[g_casino_buf]; i++) {
        p.x = g_casino_rects.pos[g_casino_buf][i].x;
        p.y = g_casino_rects.pos[g_casino_buf][i].y;
        p.w = g_casino_rects.pos[g_casino_buf][i].w;
        p.h = g_casino_rects.pos[g_casino_buf][i].h;
        t.x = g_casino_rects.uv[g_casino_buf][i].x;
        t.y = g_casino_rects.uv[g_casino_buf][i].y;
        t.w = g_casino_rects.uv[g_casino_buf][i].w;
        t.h = g_casino_rects.uv[g_casino_buf][i].h;
        k = g_casino_rects.spr[g_casino_buf][i];
        g_casino_sprites.prim[g_casino_buf][k].x0 = p.x;
        g_casino_sprites.prim[g_casino_buf][k].y0 = p.y;
        g_casino_sprites.prim[g_casino_buf][k].x1 = p.x + p.w;
        g_casino_sprites.prim[g_casino_buf][k].y1 = p.y;
        g_casino_sprites.prim[g_casino_buf][k].x2 = p.x;
        g_casino_sprites.prim[g_casino_buf][k].y2 = p.y + p.h;
        g_casino_sprites.prim[g_casino_buf][k].x3 = p.x + p.w;
        g_casino_sprites.prim[g_casino_buf][k].y3 = p.y + p.h;
        g_casino_sprites.prim[g_casino_buf][k].u0 = t.x;
        g_casino_sprites.prim[g_casino_buf][k].v0 = t.y;
        g_casino_sprites.prim[g_casino_buf][k].u1 = t.x + t.w;
        g_casino_sprites.prim[g_casino_buf][k].v1 = t.y;
        g_casino_sprites.prim[g_casino_buf][k].u2 = t.x;
        g_casino_sprites.prim[g_casino_buf][k].v2 = t.y + t.h;
        g_casino_sprites.prim[g_casino_buf][k].u3 = t.x + t.w;
        g_casino_sprites.prim[g_casino_buf][k].v3 = t.y + t.h;
        g_casino_sprites.prim[g_casino_buf][k].tpage = g_casino_rects.tpage[g_casino_buf][i];
        g_casino_sprites.prim[g_casino_buf][k].clut = g_casino_rects.clut[g_casino_buf][i];
    }
    g_casino_rects.n[g_casino_buf] = 0;
}
