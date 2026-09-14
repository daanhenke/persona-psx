/* Persona 1 (JP) - a fighter going out of the fight.  BTLP only.
 *   0x8008D2F0 BtlMemberMotion0E
 *
 * Entry 0x0E of g_btl_member_motion, named for the motion like the rest of the
 * table. It waits out the record's timer, marks the actor out, and plays the
 * fall - an enemy big enough to carry its own sound picks it by size, any other
 * enemy takes the sixth. The fighter's palette is put back to how it was loaded
 * in both the copy it fades toward and the one it is drawn with, it is started
 * sinking - enemies up the screen, members down - and it is given a mid grey to
 * fade to black from, with its shadow taken away.
 *
 * Each frame after that it keeps sinking, a little faster every time, until
 * its colour has gone. Then its cell is freed - in the enemies' grid, or in the
 * party's formation unless the member was already gone from it - the attack
 * lines are redrawn, and it, its shadow and its ailment marker are hidden.
 */
#include <decomp/types.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/formation.h>
#include <persona/btlp/object.h>
#include <persona/btlp/round.h>
#include <persona/btlp/sound.h>

/* The attribute bit on an enemy that carries a fall sound of its own. */
#define LEAVE_OWN_SOUND 0x800

/* An actor flag for a member no longer standing in the formation. */
#define ACTOR_UNPLACED 0x80000000

/* The fall every other enemy makes, and the first of the sized ones. */
#define LEAVE_SOUND      6
#define LEAVE_SOUND_SIZE 2

/* How fast it sinks to start with and how much faster each frame, 16.16, and
   the grey it fades from. */
#define LEAVE_SINK  0x10000
#define LEAVE_SPEED 0x8000
#define LEAVE_GREY  0x80
#define LEAVE_FADE  3

/* Cells of the enemies' grid to a row. */
#define ENEMY_GRID_W 9

/* One actor's palette, as the copy moves it. */
typedef struct {
    u_char b[0x200];
} BtlClutBlock;

#define ACTOR_CLUT(base, slot) ((BtlClutBlock *)((base) + (slot) * 0x200))

extern u_char *g_btl_actor_clut;
extern u_char *g_btl_actor_clut_to;
extern u_char *g_btl_actor_clut_base;
extern u_char  g_btl_grid[];

void BtlMemberMotion0E(BtlObj *o)
{
    if (o->attr & BTL_OBJ_TRAIL) {
        return;
    }
    switch (o->phase) {
    case 0:
        if (o->timer != 0) {
            break;
        }
        o->actor->flags |= BTL_ACTOR_OUT;
        if (o->attr & BTL_OBJ_OTHER_SIDE) {
            if (o->attr & LEAVE_OWN_SOUND) {
                BtlSePlay((o->unkCD >> 1) + LEAVE_SOUND_SIZE, 1);
            } else {
                BtlSePlay(LEAVE_SOUND, 1);
            }
        }
        *ACTOR_CLUT(g_btl_actor_clut_to, o->mark_num) =
            *ACTOR_CLUT(g_btl_actor_clut_base, o->mark_num);
        *ACTOR_CLUT(g_btl_actor_clut, o->mark_num) =
            *ACTOR_CLUT(g_btl_actor_clut_base, o->mark_num);
        o->step_y = (o->attr & BTL_OBJ_OTHER_SIDE) ? -LEAVE_SINK : LEAVE_SINK;
        o->rgb[0] = LEAVE_GREY;
        o->rgb[1] = LEAVE_GREY;
        o->rgb[2] = LEAVE_GREY;
        o->fade = LEAVE_FADE;
        o->rgb_to[0] = 0;
        o->rgb_to[1] = 0;
        o->rgb_to[2] = 0;
        o->attr |= BTL_OBJ_NO_SHADOW;
        o->phase++;
        break;
    case 1:
        if ((u_short)(o->rgb[0] | o->rgb[1] | o->rgb[2])) {
            o->y += o->step_y;
            o->step_y += (o->attr & BTL_OBJ_OTHER_SIDE) ? -LEAVE_SPEED
                                                      : LEAVE_SPEED;
            break;
        }
        if (o->attr & BTL_OBJ_OTHER_SIDE) {
            g_btl_grid[o->row * ENEMY_GRID_W + o->col2] = CELL_EMPTY;
            o->actor->c.key = 0;
        } else {
            if (o->actor->flags & ACTOR_UNPLACED) {
                goto hide;
            }
            g_btl_formation[o->row * GRID_W + (o->col2 >> 1)] = CELL_EMPTY;
        }
        BtlReadyNextTurn();
    hide:
        o->attr |= BTL_OBJ_HIDDEN;
        o->shadow->attr |= BTL_OBJ_HIDDEN;
        o->mark->attr |= BTL_OBJ_HIDDEN;
        o->motion = 0;
        o->phase = 0;
        break;
    }
}
