/* Persona 1 (JP) - where the room is drawn from.  ADV only.
 *   0x80083DC0 CamCenterOnActor
 *
 * g_cam_x and g_cam_y are the isometric origin the renderer subtracts from an
 * actor's world position. Entering a room puts them on the actor the camera
 * follows; the walk code scrolls them a frame at a time from there.
 *
 * The camera only tracks an actor while it is at least four tiles from either
 * edge of the room, so the view never runs past the drawn area: outside that
 * band the offset sticks at the edge tile's value.
 */
#include <types.h>
#include <persona/adv/actor.h>
#include <persona/adv/scene.h>

extern u_short g_cam_x;
extern u_short g_cam_y;

/* One tile is 21 pixels of isometric x and 7 of y, as in ActorSetTile. */
#define TILE_X 21
#define TILE_Y 7
#define EDGE   4

/* The tile is read out of the record each time rather than through a local -
   the extra byte the local costs does not fold away. */
void CamCenterOnActor(u_char actor)
{
    AdvScene *sc;
    u_char    lim_w;
    u_char    lim_h;
    u_char    d;

    sc = g_adv_scene;
    lim_w = sc->w - 3;
    lim_h = sc->h - 3;
    /* Each room kind starts the camera somewhere different. */
    switch (sc->kind) {
    case 0:
        g_cam_y = 0x1D;
        g_cam_x = 0;
        break;
    case 1:
        g_cam_y = 0x14;
        g_cam_x = 0;
        break;
    case 2:
        g_cam_y = 0x30;
        g_cam_x = 0;
        break;
    case 3:
        g_cam_y = 0x5D;
        g_cam_x = 0;
        break;
    case 4:
        g_cam_y = 0x78;
        g_cam_x = 5;
        break;
    }
    if (g_adv_actors[actor].y > EDGE) {
        if (lim_h < g_adv_actors[actor].y) {
            d = lim_h - EDGE;
        } else {
            d = g_adv_actors[actor].y - EDGE;
        }
        g_cam_y = g_cam_y + d * TILE_Y;
        g_cam_x = g_cam_x + d * TILE_X;
    }
    if (g_adv_actors[actor].x > EDGE) {
        if (lim_w < g_adv_actors[actor].x) {
            d = lim_w - EDGE;
        } else {
            d = g_adv_actors[actor].x - EDGE;
        }
        g_cam_y = g_cam_y - d * TILE_Y;
        g_cam_x = g_cam_x + d * TILE_X;
    }
}
