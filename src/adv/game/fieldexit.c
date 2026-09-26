/* Persona 1 (JP) - the party walking out through an exit.  ADV only.
 *   0x8008060C AdvExitPlaceLeader   0x80080720 ActorWalkStep
 *
 * An exit the scene pack gives a walk-out sprite has the leader drawn leaving
 * by it: the exit's sound plays, and record 24 is put on the exit's walk-out
 * tile, behind everything but the backdrop, with the sprite for that exit.
 * An exit left only upwards takes the second of the pair.
 */
#include <decomp/types.h>
#include <persona/adv/actor.h>
#include <persona/adv/scene.h>
#include <persona/adv/room.h>

#define LEADER      24
#define LEADER_SLOT 0x32
#define LEAVE_NONE  0xFF
#define LEAVE_Z     0x3BD

extern void *g_leave_defs[];

extern void AdvSoundCommand(short cmd);
extern void ActorSetTile(short x, short y, AdvActor *a);
extern void SlotInit(void *def, u_char slot, int attr, short x, short y);

void AdvExitPlaceLeader(u_char n)
{
    u_char def;

    if (g_adv_scene->entries[n].leave != LEAVE_NONE) {
        AdvSoundCommand(g_adv_scene->exit_sound);
        ActorSetTile(g_adv_scene->entries[n].leave_x,
                     g_adv_scene->entries[n].leave_y, &g_adv_actors[LEADER]);
        g_adv_actors[LEADER].z = LEAVE_Z;
        g_adv_actors[LEADER].world_y += 8;
        g_adv_actors[LEADER].id = 0;
        def = g_adv_scene->entries[n].leave * 2;
        if (g_adv_scene->entries[n].dirs == 1) {
            def |= 1;
        }
        SlotInit(g_leave_defs[def], LEADER_SLOT, LEAVE_Z,
                 g_adv_actors[LEADER].world_x, g_adv_actors[LEADER].world_y);
    }
}

#define TILE_SLOPE_V 7
#define TILE_SLOPE_H 8

extern short       g_dir_dz[];
extern signed char g_slope_lift[];  /* by facing */

extern u_char SceneTileAt(u_char x, u_char y);
extern void WalkAdvance(u_short *wy, u_short *wx, u_char dir, int phase,
                        u_char steps);
extern void WalkSlopeAdvance(u_short *wy, u_short *wx, u_char dir,
                             u_char phase, u_char steps, u_char slope);

#define A g_adv_actors[i]

/* One frame of a walk the player is steering: ActorsMoveStep's step for a
   single actor. Half way across a tile the actor passes onto the next one,
   remembering the one it left, and a slope under it lifts or lowers it; the
   camera always follows. */
void ActorWalkStep(u_char i)
{
    u_char old;
    u_char new;

    if ((A.phase & 0xF) == 8) {
        A.home_x = A.x;
        A.home_y = A.y;
        old = SceneTileAt(A.x, A.y);
        A.z += g_dir_dz[A.dir];
        A.x += g_dir_x[A.dir];
        A.y += g_dir_y[A.dir];
        new = SceneTileAt(A.x, A.y);
        if (new == TILE_SLOPE_V || new == TILE_SLOPE_H) {
            switch (A.dir) {
            case 0:
                if (new == TILE_SLOPE_V) A.slope = 0;
                if (new == TILE_SLOPE_H) {
                    A.slope = 0;
                    A.lift += g_slope_lift[A.dir];
                    if (old == TILE_SLOPE_H) A.lift += g_slope_lift[A.dir];
                }
                break;
            case 1:
                if (new == TILE_SLOPE_V) A.slope = 0;
                if (new == TILE_SLOPE_H) {
                    A.slope = 1;
                    A.lift += g_slope_lift[A.dir];
                    if (old == TILE_SLOPE_H) A.lift += g_slope_lift[A.dir];
                }
                break;
            case 2:
                if (new == TILE_SLOPE_H) A.slope = 0;
                if (new == TILE_SLOPE_V) {
                    A.slope = 2;
                    A.lift += g_slope_lift[A.dir];
                    if (old == TILE_SLOPE_V) A.lift += g_slope_lift[A.dir];
                }
                break;
            case 3:
                if (new == TILE_SLOPE_H) A.slope = 0;
                if (new == TILE_SLOPE_V) {
                    A.slope = 3;
                    A.lift += g_slope_lift[A.dir];
                    if (old == TILE_SLOPE_V) A.lift += g_slope_lift[A.dir];
                }
                break;
            }
        } else {
            if (old == TILE_SLOPE_V || old == TILE_SLOPE_H) {
                switch (A.dir) {
                case 0:
                    if (old == TILE_SLOPE_H) A.lift += g_slope_lift[0];
                    break;
                case 1:
                    if (old == TILE_SLOPE_H) A.lift += g_slope_lift[1];
                    break;
                case 2:
                    if (old == TILE_SLOPE_V) A.lift += g_slope_lift[2];
                    break;
                case 3:
                    if (old == TILE_SLOPE_V) A.lift += g_slope_lift[3];
                    break;
                }
            }
            A.slope = 0;
        }
    }
    WalkAdvance(&A.world_y, &A.world_x, A.dir, A.phase, A.steps);
    WalkSlopeAdvance(&A.world_y, &A.world_x, A.dir, A.phase, A.steps,
                     A.slope);
    CamFollowStep();
    A.phase = (A.phase + A.steps) & 0xF;
}
