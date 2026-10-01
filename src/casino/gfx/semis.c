/* Persona 1 (JP) - CASINO's queue of semi-transparency changes.
 *   0x8006A02C CasinoQueueSemi
 *   0x8006A150 CasinoFlushSemis
 *
 * Turns blending on or off for a run of sprites, applied to each buffer's
 * polygons as that buffer is next drawn. At most eight runs a frame.
 */
#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <persona/casino/casino.h>

void CasinoQueueSemi(short first, short count, u_char mode)
{
    short no;
    short nb;

    no = CASINO_OTHER(g_casino_semis.n);
    nb = g_casino_semis.n[g_casino_buf];
    CASINO_OTHER(g_casino_semis.first)[no] = first;
    g_casino_semis.first[g_casino_buf][nb] = first;
    CASINO_OTHER(g_casino_semis.count)[no] = count;
    g_casino_semis.count[g_casino_buf][nb] = count;
    CASINO_OTHER(g_casino_semis.mode)[no] = mode;
    g_casino_semis.mode[g_casino_buf][nb] = mode;
    CASINO_OTHER(g_casino_semis.n)++;
    g_casino_semis.n[g_casino_buf]++;
}

void CasinoFlushSemis(void)
{
    int    i;
    int    j;
    int    first;
    int    count;
    u_char mode;

    for (i = 0; i < g_casino_semis.n[g_casino_buf]; i++) {
        first = g_casino_semis.first[g_casino_buf][i];
        count = g_casino_semis.count[g_casino_buf][i];
        mode = g_casino_semis.mode[g_casino_buf][i];
        for (j = first; j < first + count; j++) {
            SetSemiTrans(&g_casino_sprites.prim[g_casino_buf][j], mode);
        }
    }
    g_casino_semis.n[g_casino_buf] = 0;
}
