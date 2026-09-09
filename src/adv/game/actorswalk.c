/* Persona 1 (JP) - stepping a walk in progress.  ADV only.
 *   0x800834B4 WalkAdvance
 *
 * The tail of the actor unit, past a table that is not worked out yet. This
 * adds up `steps` frames from `phase` and applies the total to a screen
 * position with the signs the facing calls for - the same projection
 * ActorSetTile uses, so a walk in progress lands on the tile ActorSetTile
 * would have given it. See persona/adv/room.h.
 */
#include <decomp/types.h>
#include <persona/adv/room.h>

void WalkAdvance(u_short *wy, u_short *wx, u_char dir, int phase, u_char steps)
{
    u_short sy;
    u_short sx;
    int     i;

    sy = 0;
    sx = 0;
    while (steps != 0) {
        steps--;
        i = phase & 0xF;
        phase = i + 1;
        sy = g_walk_dy[i] + sy;
        sx = g_walk_dx[i] + sx;
    }
    switch (dir) {
    case 0:
        *wy = *wy - sy;
        *wx = *wx - sx;
        break;
    case 1:
        *wy = sy + *wy;
        *wx = sx + *wx;
        break;
    case 2:
        *wy = sy + *wy;
        *wx = *wx - sx;
        break;
    case 3:
        *wy = *wy - sy;
        *wx = sx + *wx;
        break;
    }
}
