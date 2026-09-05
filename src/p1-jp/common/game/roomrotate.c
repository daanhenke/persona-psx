/* Persona 1 (JP) - room coordinates under a change of facing.
 *
 *   DNG 0x800961D0   ADV 0x80095854   S2D 0x80086664
 *
 * A room is a 24 by 24 grid and the view faces one of four ways, so moving a
 * point from one facing to another is a quarter turn per step: the four bodies
 * below are the identity and the three rotations, and the pair of switches
 * picks between them by how far apart the two facings are.
 */
#include <types.h>

#define ROOM_MAX 0x17   /* 24 cells across, so the far edge is 23 */


void RoomRotatePoint(short from, short x, short y, short to,
                     short *ox, short *oy)
{
    switch (from) {
    case 0:
        switch (to) {
        case 0:
            *ox = x;
            *oy = y;
            break;
        case 1:
            *ox = ROOM_MAX - y;
            *oy = x;
            break;
        case 2:
            *ox = ROOM_MAX - x;
            *oy = ROOM_MAX - y;
            break;
        case 3:
            *ox = y;
            *oy = ROOM_MAX - x;
            break;
        }
        break;
    case 1:
        switch (to) {
        case 0:
            *ox = y;
            *oy = ROOM_MAX - x;
            break;
        case 1:
            *ox = x;
            *oy = y;
            break;
        case 2:
            *ox = ROOM_MAX - y;
            *oy = x;
            break;
        case 3:
            *ox = ROOM_MAX - x;
            *oy = ROOM_MAX - y;
            break;
        }
        break;
    case 2:
        switch (to) {
        case 0:
            *ox = ROOM_MAX - x;
            *oy = ROOM_MAX - y;
            break;
        case 1:
            *ox = y;
            *oy = ROOM_MAX - x;
            break;
        case 2:
            *ox = x;
            *oy = y;
            break;
        case 3:
            *ox = ROOM_MAX - y;
            *oy = x;
            break;
        }
        break;
    case 3:
        switch (to) {
        case 0:
            *ox = ROOM_MAX - y;
            *oy = x;
            break;
        case 1:
            *ox = ROOM_MAX - x;
            *oy = ROOM_MAX - y;
            break;
        case 2:
            *ox = y;
            *oy = ROOM_MAX - x;
            break;
        case 3:
            *ox = x;
            *oy = y;
            break;
        }
        break;
    }
}
