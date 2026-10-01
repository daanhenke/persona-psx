/* Persona 1 (JP) - CASINO's tweens: layouts slid and textures scrolled.
 *   0x8006AAA8 CasinoTween
 *   0x8006AB1C CasinoAddTween
 *   0x8006ACBC CasinoStepTweens
 *   0x8006AFD0 CasinoScroll
 *   0x8006B044 CasinoAddScroll
 *   0x8006B1B0 CasinoStepScrolls
 *
 * A tween moves every piece of a layout by a rectangle spread over a number
 * of frames (a scroll does the same to the pieces' textures). The layout
 * stays busy until it ends. Each piece's running offset is kept per sprite,
 * and the frame queues the piece's base rectangle plus that offset.
 */
#include <decomp/types.h>
#include <persona/casino/casino.h>

/* Called without prototypes: the corners go over as plain ints. */
extern void CasinoQueueQuad();
extern void CasinoQueueUV();

/* The arguments, gathered into one place and handed on by value. */
extern CasinoTweenArg g_casino_tween_arg;
extern CasinoTweenArg g_casino_scroll_arg;

void CasinoAddTween(CasinoTweenArg t);
void CasinoAddScroll(CasinoTweenArg t);

void CasinoTween(CasinoLayout *l, short dx, short dy, short dw, short dh, short frames)
{
    g_casino_tween_arg.d.x = dx;
    g_casino_tween_arg.d.y = dy;
    g_casino_tween_arg.d.w = dw;
    g_casino_tween_arg.l = l;
    g_casino_tween_arg.d.h = dh;
    g_casino_tween_arg.frames = frames;
    CasinoAddTween(g_casino_tween_arg);
}

void CasinoAddTween(CasinoTweenArg t)
{
    int n;

    n = g_casino_tweens.n;
    g_casino_tweens.delta[n].x = t.d.x / t.frames;
    g_casino_tweens.delta[n].y = t.d.y / t.frames;
    g_casino_tweens.delta[n].w = t.d.w / t.frames;
    g_casino_tweens.delta[n].h = t.d.h / t.frames;
    g_casino_tweens.layout[n] = t.l;
    t.l->busy = 1;
    g_casino_tweens.left[n] = t.frames - 1;
    g_casino_tweens.n++;
}

void CasinoStepTweens(void)
{
    RECT          base;
    RECT          d;
    RECT          q;
    CasinoLayout *l;
    CasinoCell   *c;
    int           i;
    int           j;
    int           k;

    for (i = 0; i < g_casino_tweens.n; i++) {
        l = g_casino_tweens.layout[i];
        d = g_casino_tweens.delta[i];
        for (j = 0; j < l->n; j++) {
            base = *(RECT *)&l->cells[l->cell_idx[j]];
            c = g_casino_cells + l->first + j;
            c->x += d.x;
            c->y += d.y;
            c->w += d.w;
            c->h += d.h;
            q.x = c->x + base.x;
            q.y = c->y + base.y;
            q.w = c->w + base.w;
            q.h = c->h + base.h;
            CasinoQueueQuad(q.x, q.y, q.x + q.w, q.y, q.x, q.y + q.h, q.x + q.w, q.y + q.h, j + l->first);
        }
        if (--g_casino_tweens.left[i] == -1) {
            l->busy = 0;
            for (k = i; k < g_casino_tweens.n; k++) {
                g_casino_tweens.delta[k] = g_casino_tweens.delta[k + 1];
                g_casino_tweens.layout[k] = g_casino_tweens.layout[k + 1];
                g_casino_tweens.left[k] = g_casino_tweens.left[k + 1];
            }
            g_casino_tweens.n--;
            i--;
        }
    }
}

void CasinoScroll(CasinoLayout *l, short du, short dv, short dw, short dh, short frames)
{
    g_casino_scroll_arg.d.x = du;
    g_casino_scroll_arg.d.y = dv;
    g_casino_scroll_arg.d.w = dw;
    g_casino_scroll_arg.l = l;
    g_casino_scroll_arg.d.h = dh;
    g_casino_scroll_arg.frames = frames;
    CasinoAddScroll(g_casino_scroll_arg);
}

void CasinoAddScroll(CasinoTweenArg t)
{
    CasinoLayout *l;
    RECT         *r;
    short         f;
    int           n;

    l = t.l;
    n = g_casino_scrolls.n;
    r = &g_casino_scrolls.delta[n];
    r->x = t.d.x / (f = t.frames);
    r->y = t.d.y / f;
    r->w = t.d.w / f;
    r->h = t.d.h / f;
    g_casino_scrolls.layout[n] = l;
    g_casino_scrolls.left[n] = f - 1;
    l->busy = 1;
    g_casino_scrolls.n++;
}

void CasinoStepScrolls(void)
{
    RECT          base;
    RECT          d;
    RECT          q;
    CasinoLayout *l;
    CasinoTex    *c;
    CasinoTex    *p;
    u_short       tp;
    u_short       cl;
    int           i;
    int           j;
    int           k;
    short         n;
    short         t;

    for (i = 0; i < g_casino_scrolls.n; i++) {
        l = g_casino_scrolls.layout[i];
        n = l->n;
        d = g_casino_scrolls.delta[i];
        for (j = 0; j < n; j++) {
            t = l->tex_idx[j];
            base = *(RECT *)&l->tex[t];
            c = g_casino_texs + l->first + j;
            c->u += d.x;
            c->v += d.y;
            c->w += d.w;
            c->h += d.h;
            q.x = base.x + c->u;
            q.y = base.y + c->v;
            q.w = base.w + c->w;
            q.h = base.h + c->h;
            p = &l->tex[t];
            tp = p->tpage;
            cl = p->clut;
            CasinoQueueUV(q.x, q.y, q.w, q.h, tp, cl, j + l->first);
        }
        if (--g_casino_scrolls.left[i] == -1) {
            l->busy = 0;
            for (k = i; k < g_casino_scrolls.n; k++) {
                g_casino_scrolls.delta[k] = g_casino_scrolls.delta[k + 1];
                g_casino_scrolls.layout[k] = g_casino_scrolls.layout[k + 1];
                g_casino_scrolls.left[k] = g_casino_scrolls.left[k + 1];
            }
            g_casino_scrolls.n--;
            i--;
        }
    }
}
