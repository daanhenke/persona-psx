/* Persona 1 (JP) - CASINO's palette animations.
 *   0x8006C2C4 CasinoStartPalAnim
 *   0x8006C330 CasinoStepPalAnims
 *   0x8006C540 CasinoStartPalCycle
 *   0x8006C5B0 CasinoStepPalCycles
 *
 * Lights and flashing signs: a CLUT in VRAM is rewritten every few frames,
 * either from a sequence of prepared palettes or by rotating one palette's
 * colours, and every sprite drawn with it changes at once. Up to eight of
 * each run at a time; a finished one switches itself off.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <libgte.h>
#include <libgpu.h>
#include <persona/casino/casino.h>

extern void CasinoQueueImage(u_long *data, RECT *r);

/* The rotated palette, built here before it is queued. */
extern u_short g_casino_pal_buf[16];

void CasinoStartPalAnim(CasinoPalAnim *a)
{
    int n;

    n = g_casino_palanims.n;
    g_casino_palanims.a[n] = a;
    g_casino_palanims.len[n] = a->frames + a->wait * a->frames;
    g_casino_palanims.t[n] = 0;
    g_casino_palanims.n++;
    a->on = 1;
}

void CasinoStepPalAnims(void)
{
    CasinoPalAnim *a;
    short          len;
    int            i;
    int            k;
    char           frame_step;

    for (i = 0; g_casino_palanims.n > i; i++) {
        k = i;
        a = g_casino_palanims.a[k];
        len = g_casino_palanims.len[i];
        if (g_casino_palanims.t[i] % (a->wait + 1) == 0) {
            frame_step = 1;
            CasinoQueueImage(a->pal[a->seq[g_casino_palanims.t[i] / (a->wait + frame_step)]], &a->r);
        }
        if (g_casino_palanims.t[i]++ == len - 1) {
            a->on = 0;
            for (k = i; k < g_casino_palanims.n; k++) {
                g_casino_palanims.a[k] = g_casino_palanims.a[k + 1];
                g_casino_palanims.len[k] = g_casino_palanims.len[k + 1];
                g_casino_palanims.t[k] = g_casino_palanims.t[k + 1];
            }
            g_casino_palanims.n--;
            i--;
        }
    }
}

void CasinoStartPalCycle(CasinoPalCycle *c)
{
    int n;

    n = g_casino_palcycles.n;
    g_casino_palcycles.a[n] = c;
    g_casino_palcycles.len[n] = c->count + c->wait * c->count;
    g_casino_palcycles.t[n] = 0;
    g_casino_palcycles.n++;
    c->on = 1;
}

/* The rotation and expiry paths reuse their integer temporaries. Keep the
 * previous timer in a short before comparing it with the cycle length. */
void CasinoStepPalCycles(void)
{
    CasinoPalCycle *c;
    short           len;
    int             i;
    int             j;
    int             k;
    int             q;
    int             ahead;
    int             t;
    int             w;
    short           n;
    short           old_timer;
    int             timer_raw;

    for (i = 0; i < g_casino_palcycles.n; i++) {
        c = g_casino_palcycles.a[i];
        len = g_casino_palcycles.len[i];
        n = c->count;
        if (g_casino_palcycles.t[i] % (c->wait + 1) == 0) {
            t = g_casino_palcycles.t[i];
            w = c->wait + 1;
            for (j = 0; j < n; j += 2) {
                k = t;
                timer_raw = w;
                q = k / timer_raw;
                ahead = q + 1;
                g_casino_pal_buf[(j + ahead) % n] = c->pal[j + 1];
                g_casino_pal_buf[(j + q) % n] = c->pal[j];
            }
            CasinoQueueImage((u_long *)g_casino_pal_buf, &c->r);
        }
        k = i;
        timer_raw = g_casino_palcycles.t[i]++;
        old_timer = timer_raw;
        if (old_timer == len - 1) {
            c->on = 0;
            for (k = i; k < g_casino_palcycles.n; k++) {
                g_casino_palcycles.a[k] = g_casino_palcycles.a[k + 1];
                g_casino_palcycles.len[k] = g_casino_palcycles.len[k + 1];
                g_casino_palcycles.t[k] = g_casino_palcycles.t[k + 1];
            }
            g_casino_palcycles.n--;
            i--;
        }
    }
}
