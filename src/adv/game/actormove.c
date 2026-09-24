/* Persona 1 (JP) - scripted actor moves.  ADV only.
 *   0x800AE300 ActorStartMove    0x800AE3C8 ActorsMoveStep
 *
 * The scene pack carries a table of move scripts at 0xF0: each is a run of
 * (dir, tiles, wait) triples ended by 0xFF. Script command 6E sets one going
 * on an actor, and ActorsMoveStep walks every actor's a frame at a time.
 *
 * A move is taken a tile at a time. Each move begins by facing the actor, and
 * an actor of the stepping kind is given its walking sprite then; a slope
 * tile - 7 climbs going left or right, 8 going up or down - lifts the actor
 * as it passes the middle of a tile.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <persona/adv/actor.h>
#include <persona/adv/room.h>
#include <persona/common/slot.h>

#define g_slots ((Slot *)0x800DC10C)

/* The pack's move scripts, reached by address. */
#define g_move_scripts ((u_char **)0x801000F0)
#define g_seq_handle   (*(short *)0x801F5390)

/* An actor of this kind has footsteps and a walking sprite. */
#define KIND_STEPS 1

#define MOVE_END    0xFF
#define SHADOW_SLOT 24

#define TILE_SLOPE_V 7
#define TILE_SLOPE_H 8

extern Slot        *g_slot_cur;
extern void        *g_walk_defs[];   /* by facing; +4 for the shadow's */
extern u_char       g_dir_flip[];
extern short        g_dir_dz[];
extern signed char  g_slope_lift[];  /* by facing                      */

extern void   SsSeqPlay(short seq, char mode, short times);
extern void   SlotInit(void *def, u_char slot, int attr, short x, short y);
/* Called without a prototype here: the tile comes back as an int. */
extern int    SceneTileAt();
extern void   WalkAdvance(u_short *wy, u_short *wx, u_char dir, int phase,
                          u_char steps);
extern void   WalkSlopeAdvance(u_short *wy, u_short *wx, u_char dir,
                               u_char phase, u_char steps, u_char slope);
extern void   ActorSetStandSprite(u_char a);

/* Starts actor `a` on move script `move`: `speed` more than one walk phase a
   frame, facing `dir` once it is done, and turning its sprite with each move
   if `face_moves` says so. */
void ActorStartMove(u_char a, u_char move, u_char speed, u_char dir,
                    u_char face_moves)
{
    g_adv_actors[a].move = g_move_scripts[move];
    g_adv_actors[a].face = g_adv_actors[a].dir;
    g_adv_actors[a].tiles_left = 0;
    g_adv_actors[a].steps = speed + 1;
    g_adv_actors[a].next_dir = dir;
    g_adv_actors[a].face_moves = face_moves;
    if (g_adv_actors[a].kind == KIND_STEPS) {
        SsSeqPlay(g_seq_handle, 1, 0);
    }
}

#define A g_adv_actors[i]

/* 99.54%: storing the index into clut_y (a halfword extension of it) makes
   cse drop the word extension it already has, so the actor record's index
   is masked again before the shadow test; the original keeps the one
   register. Moving, casting or inlining the store does not change it. */
#ifdef NON_MATCHING
/* Runs every actor's move script a frame; `follow` is the actor the camera
   walks with. Answers whether any actor still has a move going. */
u_char ActorsMoveStep(u_char follow)
{
    u_char  i;
    u_char  moving;
    u_char *p;
    u_char  old;
    u_char  new;
    Slot   *s;
    Slot   *t;

    moving = 0;
    for (i = 0; i < 24; i++) {
        if (A.id == ACTOR_NONE) continue;
        if (A.move == MOVE_NONE) continue;
        moving = 1;
        if (A.tiles_left == 0) {
            if (A.wait == 0) {
            p = A.move;
            if (*p == MOVE_END) {
                A.move = MOVE_NONE;
                A.dir = A.next_dir;
                A.face = A.next_dir;
                if (A.kind == KIND_STEPS) {
                    ActorSetStandSprite(i);
                }
                continue;
            }
            A.dir = *p;
            if (A.face_moves) {
                A.face = *p;
            }
            if (A.kind == KIND_STEPS) {
                SlotInit(g_walk_defs[A.face], i, A.z, A.world_x, A.world_y);
                switch (A.shadow) {
                case SHADOW_FLAT:
                    SlotInit(g_walk_defs[A.face + 4], i + SHADOW_SLOT, A.z,
                             A.world_x, A.world_y);
                    break;
                case SHADOW_FLAT_LOW:
                    SlotInit(g_walk_defs[A.face], i + SHADOW_SLOT, A.z,
                             A.world_x, A.world_y);
                    g_slot_cur = &g_slots[i + SHADOW_SLOT];
                    g_slot_cur->clut_y = i;
                    break;
                }
                s = &g_slots[i];
                t = &g_slots[i + SHADOW_SLOT];
                s->clut_y = i;
                g_slot_cur = s;
                g_slot_cur->tpage_add = i;
                g_slot_cur = t;
                g_slot_cur->tpage_add = i;
                switch (A.shadow) {
                case SHADOW_FLAT:
                case SHADOW_FLAT_LOW:
                    if (g_dir_flip[A.face]) {
                        g_slot_cur = s;
                        g_slot_cur->attr |= SLOT_ATTR_XSCALE;
                        g_slot_cur = &g_slots[SHADOW_SLOT];
                        g_slot_cur->attr |= SLOT_ATTR_XSCALE;
                    }
                }
            }
            p++;
            A.tiles_left = *p;
            A.wait = p[1];
            A.move += 3;
            continue;
            } else {
                if (A.kind == KIND_STEPS) {
                    ActorSetStandSprite(i);
                }
                A.wait--;
                continue;
            }
        }

        if ((A.phase & 0xF) == 8) {
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
        if (i == follow) {
            CamFollowStep();
        }
        A.phase = (A.phase + A.steps) & 0xF;
        if (A.phase == 0) {
            A.tiles_left--;
        }
    }
    return moving;
}
#else
INCLUDE_ASM("adv/nonmatchings/game/actormove", ActorsMoveStep);
#endif
