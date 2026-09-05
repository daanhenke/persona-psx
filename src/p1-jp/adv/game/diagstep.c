/* Persona 1 (JP) - rounding a corner in the field scenes.
 *
 *   SceneTryDiagCW @ 0x8007FC88   SceneTryDiagCCW @ 0x8007FFD0
 *
 * The player walks on a grid, but pressing into a wall does not simply stop
 * them. The run loop asks these two whether the diagonal an eighth turn either
 * side of the walk direction is open - clockwise first, then anticlockwise -
 * and takes the first that says yes. That is what lets the player slide around
 * a corner instead of catching on it.
 *
 * A diagonal is two orthogonal legs. Facing up, the clockwise diagonal is
 * up-right: the first leg goes right, the second goes up, and the walk
 * direction ends up right - a quarter turn, because the actor carries on along
 * the second leg. Each leg has its own tile to test and its own trigger square,
 * and both have to be clear.
 *
 * The tile kinds a leg can hold:
 *
 *      3, 5    open, nothing to check
 *      2       open unless a trigger on the square refuses it
 *      4       as 2, and only if this leg's gate is set
 *      6, 9    open only if this leg's gate is set
 *      7       only while the walk is running horizontally
 *      8       only while it is running vertically
 *      10      never
 *
 * Kinds 7 and 8 also want the tile underfoot to be the same kind on the first
 * leg, so a square of that sort is only crossable by a walk already running
 * along it.
 *
 * The gate is the tile the actor is standing on for some arms and the actor's
 * own +0x26 for others, and the two routines do not agree on which goes where.
 * The jump tables are what say so; there is no pattern to lean on.
 */
#include <types.h>
#include <persona/adv/actor.h>

extern u_short g_adv_walk_dir;

extern u_char SceneTileAt(u_char x, u_char y);
extern u_char SceneTileToward(u_char x, u_char y, u_char dir);
extern u_char SceneFindTrigger(u_char x, u_char y);
extern u_char SceneTriggerArmed(u_char trigger);

u_char SceneTryDiagCW(AdvActor *a)
{
    u_char here, t1, t2;
    int blocked;
    u_char x1, y1, y2, x2;
    u_char newdir;

    here = SceneTileAt(a->x, a->y);
    switch (g_adv_walk_dir) {
    case 0:
        t1 = SceneTileToward(a->x, a->y, 3);
        newdir = 3;
        t2 = SceneTileToward(a->x + 1, a->y, 0);
        x1 = a->x + 1;
        y1 = a->y;
        x2 = x1;
        y2 = y1 - 1;
        break;
    case 1:
        t1 = SceneTileToward(a->x, a->y, 2);
        newdir = 2;
        t2 = SceneTileToward(a->x - 1, a->y, 1);
        x1 = a->x - 1;
        y1 = a->y;
        x2 = x1;
        y2 = y1 + 1;
        break;
    case 2:
        t1 = SceneTileToward(a->x, a->y, 0);
        newdir = 0;
        t2 = SceneTileToward(a->x, a->y - 1, 2);
        x1 = a->x;
        y1 = a->y - 1;
        x2 = x1 - 1;
        y2 = y1;
        break;
    case 3:
        t1 = SceneTileToward(a->x, a->y, 1);
        newdir = 1;
        t2 = SceneTileToward(a->x, a->y + 1, 3);
        x1 = a->x;
        y1 = a->y + 1;
        x2 = x1 + 1;
        y2 = y1;
        break;
    }
    switch (t1) {
    case 8:
        if (g_adv_walk_dir < 2) {
            if (here != 8) {
                return 0xFF;
            }
        }
        break;
    case 7:
        if ((u_int)(g_adv_walk_dir - 2) < 2) {
            if (here != 7) {
                return 0xFF;
            }
        }
        break;
    case 6:
    case 9:
        if (here == 0) {
            return 0xFF;
        }
        break;
    case 4:
        if (here == 0) {
            return 0xFF;
        }
    case 2:
        if (SceneTriggerArmed(SceneFindTrigger(x1, y1))) {
            return 0xFF;
        }
        break;
    case 10:
        return 0xFF;
    }
    switch (t2) {
    case 8:
        blocked = (u_int)(g_adv_walk_dir - 2) < 2;
        break;
    case 7:
        blocked = g_adv_walk_dir < 2;
        break;
    case 6:
    case 9:
        if (a->unk26 == 0) {
            return 0xFF;
        }
        goto move;
    case 4:
        if (a->unk26 == 0) {
            return 0xFF;
        }
    case 2:
        blocked = SceneTriggerArmed(SceneFindTrigger(x2, y2));
        break;
    case 10:
        return 0xFF;
    default:
        goto move;
    }
    if (blocked) {
        return 0xFF;
    }
move:
    g_adv_walk_dir = newdir;
    return 0;
}

u_char SceneTryDiagCCW(AdvActor *a)
{
    u_char here, t1, t2;
    int blocked;
    u_char x1, y1, y2, x2;
    u_char newdir;

    here = SceneTileAt(a->x, a->y);
    switch (g_adv_walk_dir) {
    case 0:
        t1 = SceneTileToward(a->x, a->y, 2);
        newdir = 2;
        t2 = SceneTileToward(a->x - 1, a->y, 0);
        x1 = a->x - 1;
        y1 = a->y;
        x2 = x1;
        y2 = y1 - 1;
        break;
    case 1:
        t1 = SceneTileToward(a->x, a->y, 3);
        newdir = 3;
        t2 = SceneTileToward(a->x + 1, a->y, 1);
        x1 = a->x + 1;
        y1 = a->y;
        x2 = x1;
        y2 = y1 + 1;
        break;
    case 2:
        t1 = SceneTileToward(a->x, a->y, 1);
        newdir = 1;
        t2 = SceneTileToward(a->x, a->y + 1, 2);
        x1 = a->x;
        y1 = a->y + 1;
        x2 = x1 - 1;
        y2 = y1;
        break;
    case 3:
        t1 = SceneTileToward(a->x, a->y, 0);
        newdir = 0;
        t2 = SceneTileToward(a->x, a->y - 1, 3);
        x1 = a->x;
        y1 = a->y - 1;
        x2 = x1 + 1;
        y2 = y1;
        break;
    }
    switch (t1) {
    case 8:
        if (g_adv_walk_dir < 2) {
            if (here != 8) {
                return 0xFF;
            }
        }
        break;
    case 7:
        if ((u_int)(g_adv_walk_dir - 2) < 2) {
            if (here != 7) {
                return 0xFF;
            }
        }
        break;
    case 6:
    case 9:
        if (a->unk26 == 0) {
            return 0xFF;
        }
        break;
    case 4:
        if (here == 0) {
            return 0xFF;
        }
    case 2:
        if (SceneTriggerArmed(SceneFindTrigger(x1, y1))) {
            return 0xFF;
        }
        break;
    case 10:
        return 0xFF;
    }
    switch (t2) {
    case 8:
        blocked = (u_int)(g_adv_walk_dir - 2) < 2;
        break;
    case 7:
        blocked = g_adv_walk_dir < 2;
        break;
    case 6:
    case 9:
        if (a->unk26 == 0) {
            return 0xFF;
        }
        goto move;
    case 4:
        if (here == 0) {
            return 0xFF;
        }
    case 2:
        blocked = SceneTriggerArmed(SceneFindTrigger(x2, y2));
        break;
    case 10:
        return 0xFF;
    default:
        goto move;
    }
    if (blocked) {
        return 0xFF;
    }
move:
    g_adv_walk_dir = newdir;
    return 0;
}
