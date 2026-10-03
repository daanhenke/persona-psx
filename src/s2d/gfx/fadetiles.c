/* Persona 1 (JP) - the fade's tiles.  S2D only.
 *   0x80093124 FadeTilesInit  0x80093230 FadeTilesDraw
 *   0x80093418 CopyLongs
 *
 * A semi-transparent tile and a one-pixel tile, each built once and copied
 * into its second buffer: the minimap's panel and the dot that blinks on it
 * where the party stands.
 */
#include <decomp/types.h>
#include <libgte.h>
#include <decomp/include_asm.h>
#include <libgpu.h>
#include <libgs.h>
#include <persona/s2d/s2d.h>

extern TILE   g_fade_tile[2];
extern TILE_1 g_fade_dot[2];

/* The party's tile. */
extern short D_800B8FD4;
extern short D_800B8FD8;

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

#ifdef NON_MATCHING
/* The dot steps through seven greys, at the party's tile scaled onto the
   panel; both go behind the map. */
void FadeTilesDraw(void)
{
    TILE_1 *p;
    u_char  unused[0x18];

    g_fade_dot[g_draw_side].r0 = (g_s2d_frame % 7) * 16 + 0x80;
    g_fade_dot[g_draw_side].g0 = (g_s2d_frame % 7) * 16 + 0x80;
    g_fade_dot[g_draw_side].b0 = (g_s2d_frame % 7) * 16 + 0x80;
    p = &g_fade_dot[g_draw_side];
    p->x0 = D_800B8FD4 / 2 - 0xF0;
    p->y0 = (0xC7 - D_800B8FD8) / 4 + 0x18;
    AddPrim(g_ot_back[g_draw_side].tag, p);
    AddPrim(g_ot_back[g_draw_side].tag, &g_fade_tile[g_draw_side]);
}
#else
/* 99.75%: the image keeps 0xC7 in v0 and the party's y in v1, this cc1 the
   other way round (its constant pseudo is created after the load). */
INCLUDE_ASM("s2d/nonmatchings/gfx/fadetiles", FadeTilesDraw);
#endif

/* n bytes, a word at a time (vram.c's CopyWords counts words). */
void CopyLongs(u_long *dst, u_long *src, int n)
{
    int i;

    for (i = 0; i < n / 4; i++) {
        *dst++ = *src++;
    }
}
