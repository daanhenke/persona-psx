/* Persona 1 (JP) - the fade's tiles.  S2D only.
 *   0x80093124 FadeTilesInit
 *
 * A semi-transparent tile and a one-pixel tile, each built once and copied
 * into its second buffer.
 */
#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>

extern TILE   g_fade_tile[2];
extern TILE_1 g_fade_dot[2];

void FadeTilesInit(void)
{
    SetTile(&g_fade_tile[0]);
    SetSemiTrans(&g_fade_tile[0], 1);
    SetShadeTex(&g_fade_tile[0], 0);
    g_fade_tile[0].r0 = 0xC0;
    g_fade_tile[0].g0 = 0xC0;
    g_fade_tile[0].b0 = 0x60;
    g_fade_tile[0].x0 = -0xF0;
    g_fade_tile[0].y0 = 0x18;
    g_fade_tile[0].w = 0x64;
    g_fade_tile[0].h = 0x32;
    g_fade_tile[1] = g_fade_tile[0];
    SetTile1(&g_fade_dot[0]);
    SetSemiTrans(&g_fade_dot[0], 0);
    SetShadeTex(&g_fade_dot[0], 0);
    g_fade_dot[1] = g_fade_dot[0];
}
