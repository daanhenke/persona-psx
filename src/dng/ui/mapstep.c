/* Persona 1 (JP) - the map screen's frame.
 *   0x80096818 MapScreenStep
 *
 * Once the map has finished scrolling to a tile, the page buttons turn it a
 * quarter either way - the compass, the map, the player's marker and the
 * cursor all redrawn in the new orientation - and otherwise the cursor
 * moves. The view then eases eight pixels a frame towards the cursor, and
 * backing out ends the screen.
 */
/* ADV builds this too (MAPSTEP_ADV), with the input tests' u_char results. */
/* S2D's copy takes the narrow declarations, as ADV's (MAPSTEP_NARROW). */
#if !defined(MAPSTEP_ADV) && !defined(MAPSTEP_NARROW)
#define PERSONAPAGE_DNG
#endif
#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <persona/common/menuctx.h>
#include <persona/adv/personapage.h>

#define TURNS 4

extern short   D_8009FDFC;
extern short   D_8009FE08;
extern short   D_8009FE0C;
extern short   g_map_view_room;
extern short   g_persona_data_step;
extern u_char  g_menu_allow_hold;
extern u_short g_key_page_back;
extern u_short g_key_page_fwd;
extern int     g_pad_pressed[];
extern short   g_header_scroll_x;
extern short   g_header_scroll_y;
extern short   g_map_scroll_x;
extern short   g_map_scroll_y;

extern int  MsgStep(void);
extern void MapDrawMarkers(void);
extern void DrawCompass(short facing);
extern void MapPlaceMarker(short map_dir, short player_dir, short x, short y,
                           int unused);
extern void RoomRotatePoint(short from, short x, short y, short to,
                            short *ox, short *oy);
extern void MapDrawTurned(short map, short turn);

/* A quarter turn by `d`: the two copies share their tail in the image. */
#define MAP_TURN(d)                                                                g_menu->grid[1].cur = (g_menu->grid[1].cur + (d)) & (TURNS - 1);               DrawCompass(g_menu->grid[1].cur);                                              MapDrawTurned(g_map_view_room, g_menu->grid[1].cur);                                RoomRotatePoint(n, g_menu->list[1].cur, g_menu->list[0].cur,                                   g_menu->grid[1].cur, &pt[0], &pt[1]);                          MapPlaceMarker(g_menu->grid[1].cur, D_8009FDFC, D_8009FE08, D_8009FE0C,                       0);                                                             MenuListInit(&g_menu->list[1], pt[0], 0, 0x17, 0x18);                          MenuListInit(&g_menu->list[0], pt[1], 0, 0x17, 0x14);                          g_map_scroll_y = g_header_scroll_y = g_menu->list[0].cur * 16;                 g_map_scroll_x = g_header_scroll_x = g_menu->list[1].cur * 16

void MapScreenStep(void)
{
    short pt[2];
    int   n;

    /* The facing before any turn; then each axis's target in pixels. */
    n = g_menu->grid[1].cur;
    MsgStep();
    if (!(g_map_scroll_y & 0xF) && !(g_map_scroll_x & 0xF)) {
        if (g_key_page_back & g_pad_pressed[0]) {
            MAP_TURN(-1);
        } else if (g_key_page_fwd & g_pad_pressed[0]) {
            MAP_TURN(1);
        } else if (!MenuStepCursor(&g_menu->list[0])) {
            MenuStepCursor(&g_menu->list[1]);
        }
    }
    n = g_menu->list[0].cur * 16;
    if (n < g_map_scroll_y) {
        g_map_scroll_y -= 8;
        g_header_scroll_y -= 8;
    }
    if (g_map_scroll_y < n) {
        g_map_scroll_y += 8;
        g_header_scroll_y += 8;
    }
    n = g_menu->list[1].cur * 16;
    if (n < g_map_scroll_x) {
        g_map_scroll_x -= 8;
        g_header_scroll_x -= 8;
    }
    if (g_map_scroll_x < n) {
        g_map_scroll_x += 8;
        g_header_scroll_x += 8;
    }
    MapDrawMarkers();
    if (InputCheckAcceptB(1) || g_menu_allow_hold) {
        g_persona_data_step = 0xFF;
    }
}
