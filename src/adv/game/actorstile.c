/* Persona 1 (JP) - putting an actor on a tile.  ADV only.
 *   0x80083310 ActorSetTile   0x800833B4 ActorSetTileFacing
 *
 * The middle of the actor unit: the routine before this one is not worked
 * out yet, so ADV takes that stretch from asm. The rest of the unit is in
 * actors.c and actorswalk.c, and the projection this uses is in
 * persona/adv/room.h.
 */
#include <decomp/types.h>
#include <persona/adv/actor.h>
#include <persona/adv/room.h>

/* Puts an actor on a tile and works out where that lands on screen. A tile
   nearer the camera gets the larger sort depth. */
void ActorSetTile(short x, short y, AdvActor *a)
{
    short   d;
    u_short ox;
    u_short oy;

    ox = g_room_origin_x + x * TILE_X;
    d = DEPTH_BASE - y;
    d = d - (ROOM_W - x) * 32;
    oy = g_room_origin_y;
    a->y = y;
    a->z = d;
    a->x = x;
    a->phase = 0;
    a->world_x = ox + y * TILE_X + y / 2;
    a->world_y = oy + y * TILE_Y - x * TILE_Y;
}

/* ActorSetTile that also turns the actor to face `dir`, both the way it
   faces now and the way its next step will. */
void ActorSetTileFacing(short x, short y, u_char dir, AdvActor *a)
{
    short   d;
    u_short ox;
    u_short oy;

    ox = g_room_origin_x + x * TILE_X;
    d = DEPTH_BASE - y;
    d = d - (ROOM_W - x) * 32;
    oy = g_room_origin_y;
    a->y = y;
    a->z = d;
    a->x = x;
    a->phase = 0;
    a->dir = dir;
    a->next_dir = dir;
    a->world_x = ox + y * TILE_X + y / 2;
    a->world_y = oy + y * TILE_Y - x * TILE_Y;
}
