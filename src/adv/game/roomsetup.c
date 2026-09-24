/* Persona 1 (JP) - the room's backdrop and the camera that follows a walk.
 * ADV only.
 *   0x800837E8 AdvRoomBgInit   0x80083A18 CamFollowStep
 *
 * The room is one picture drawn through BG layer 0 as a map of 16x16 cells:
 * ImageIndexInit writes the index and ImageCellsInit the cells. Each room kind
 * has its own map size and its own origin for tile (0, 0); the one colour
 * behind the picture comes from the scene record.
 *
 * While the followed actor walks, the camera moves with it - but only in the
 * band at least four tiles from each edge, the band CamCenterOnActor clamps
 * to. On the band's edge tile only half the step scrolls: the half that
 * crosses into the band.
 */
#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <persona/common/bg.h>
#include <persona/adv/actor.h>
#include <persona/adv/room.h>
#include <persona/adv/scene.h>

/* The backdrop's palette: every entry the one colour, bar the clear one. */
#define ROOM_CLUT     ((u_short *)0x800EB14C)
#define IMG_INDEX     ((u_short *)0x800EB788)
#define IMG_CELLS     ((GsCELL *)0x800E964C)

#define EDGE 4

typedef struct {
    u_short y, x;
} RoomOrigin;

typedef struct {
    u_char w, h;
} RoomMapSize;

extern RoomOrigin  g_room_origins[];
extern u_short     g_room_backdrops[];
extern RoomMapSize g_room_map_sizes[];
extern u_char      g_room_anim[];

extern u_short g_cam_x;
extern u_short g_cam_y;

extern void BgReset(void);
extern void ImageCellsInit(short kind);
extern void ImageAnimStart();

void AdvRoomBgInit(void)
{
    u_short  *clut = ROOM_CLUT;
    RECT      rect = { 0, 0x1E5, 0x100, 1 };
    int       unused[4];
    GsCELL   *cells;
    AdvScene *sc;
    u_short   c;
    short     i;

    c = g_room_backdrops[g_adv_scene->backdrop];
    clut[0] = 0;
    for (i = 1; i < 0x100; i++) {
        clut[i] = c;
    }
    LoadImage(&rect, (u_long *)ROOM_CLUT);
    DrawSync(0);
    BgReset();
    g_bg_layers[0].attribute = 0x1000000;
    /* The cell base is taken here and stored further down. */
    cells = IMG_CELLS;
    g_bg_map0.index = IMG_INDEX;
    sc = g_adv_scene;
    g_bg_layers[0].w = 0x140;
    g_bg_layers[0].x = 0;
    g_bg_layers[0].y = 0;
    g_bg_layers[0].h = 0xF0;
    g_bg_map0.base = cells;
    g_bg_map0.cellw = 16;
    g_bg_map0.cellh = 16;
    g_bg_map0.ncellw = g_room_map_sizes[sc->kind].w;
    g_bg_map0.ncellh = g_room_map_sizes[sc->kind].h;
    g_bg_layers[4].x = 0x28;
    g_bg_layers[4].y = 0xA4;
    g_bg_layers[4].h = 0x30;
    g_bg_layer_otz[0] = 0x3BE;
    g_bg_layers[4].w = 0xF0;
    g_bg_layer_otz[1] = 0x3BF;
    g_bg_layer_otz[4] = 0;
    g_room_origin_y = g_room_origins[sc->kind].y;
    g_room_origin_x = g_room_origins[sc->kind].x;
    ImageCellsInit(sc->kind);
    ImageAnimStart(1, g_room_anim, 0x111, 0x1EA, 9, 1);
    VSync(0);
}

void CamFollowStep(void)
{
    AdvScene *sc;
    u_char    lim_w;
    u_char    lim_h;
    u_char    v;
    u_char    phase;
    u_char    steps;
    u_char    dir;
    u_short   dx;
    u_short   dy;

    sc = g_adv_scene;
    lim_w = sc->w - 3;
    lim_h = sc->h - 3;
    switch (g_adv_actors[g_cam_actor].dir) {
    case 0:
        v = g_adv_actors[g_cam_actor].y;
        if (v > EDGE && v < lim_h) break;
        if (v == EDGE && g_adv_actors[g_cam_actor].phase >= 8) break;
        if (v == lim_h && g_adv_actors[g_cam_actor].phase < 8) break;
        return;
    case 1:
        v = g_adv_actors[g_cam_actor].y;
        if (v > EDGE && v < lim_h) break;
        if (v == EDGE && g_adv_actors[g_cam_actor].phase < 8) break;
        if (v == lim_h && g_adv_actors[g_cam_actor].phase >= 8) break;
        return;
    case 2:
        v = g_adv_actors[g_cam_actor].x;
        if (v > EDGE && v < lim_w) break;
        if (v == EDGE && g_adv_actors[g_cam_actor].phase >= 8) break;
        if (v == lim_w && g_adv_actors[g_cam_actor].phase < 8) break;
        return;
    case 3:
        v = g_adv_actors[g_cam_actor].x;
        if (v > EDGE && v < lim_w) break;
        if (v == EDGE && g_adv_actors[g_cam_actor].phase < 8) break;
        if (v == lim_w && g_adv_actors[g_cam_actor].phase >= 8) break;
        return;
    default:
        return;
    }

    dy = 0;
    phase = g_adv_actors[g_cam_actor].phase;
    steps = g_adv_actors[g_cam_actor].steps;
    dir = g_adv_actors[g_cam_actor].dir;
    dx = 0;
    while (steps != 0) {
        steps--;
        phase &= 0xF;
        dy += g_walk_dy[phase];
        dx += g_walk_dx[phase];
        phase++;
    }
    switch (dir) {
    case 0:
        g_cam_y -= dy;
        g_cam_x -= dx;
        break;
    case 1:
        g_cam_y += dy;
        g_cam_x += dx;
        break;
    case 2:
        g_cam_y += dy;
        g_cam_x -= dx;
        break;
    case 3:
        g_cam_y -= dy;
        g_cam_x += dx;
        break;
    }
}
