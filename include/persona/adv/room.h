#ifndef PERSONA_ADV_ROOM_H
#define PERSONA_ADV_ROOM_H

/* Persona 1 (JP) - how a room tile maps to the screen.  ADV only.
 *
 * The room is drawn isometrically: one tile step is 21 pixels of x and 7 of y,
 * with a half-pixel of x carried per row. The camera and the actor placement
 * both project with these, which is why they live here rather than in either
 * source - they were the same two numbers written out twice.
 *
 * A tile nearer the camera sorts in front, so the depth counts down from
 * DEPTH_BASE as y grows and as x falls away from the far edge.
 */
#include <decomp/types.h>

#define TILE_X     21
#define TILE_Y     7
#define DEPTH_BASE 0x440
#define ROOM_W     0x20

/* Where the room's tile (0, 0) sits on screen. */
extern u_short g_room_origin_x;
extern u_short g_room_origin_y;

/* One entry per facing: the tile step taken in each direction. */
extern const u_char g_dir_x[];
extern const u_char g_dir_y[];

/* Sixteen frames of walk animation carry an actor exactly one tile: these sum
   to TILE_X and TILE_Y over a full cycle. */
extern const u_char g_walk_dx[];
extern const u_char g_walk_dy[];

#endif
