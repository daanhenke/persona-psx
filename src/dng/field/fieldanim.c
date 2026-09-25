/* Persona 1 (JP) - the field's small per-frame animations.  DNG only.
 *   0x800676DC FieldBobVerts
 *   0x80067798 FieldFadePrims
 *   0x8006783C FieldClockTick
 *   0x80067A74 FieldDoorSlide
 *   0x80067B68 FieldClockHands
 *
 * Everything the field loop moves on its own each frame, whatever the player
 * does: bobbing vertices, primitives fading down and snapping back, a door
 * sliding open, and the two clocks - the save's play clock and the countdown
 * some floors run against, whose hand the floor's clock face shows.
 */
#include <decomp/types.h>
#include <persona/dng/field.h>

/* The hours of either clock are bumped through a literal address held in a
   register from the top of its block, as btlp's BtlClockTick does; the
   compare at 99 hours still goes through the symbol. */
#define PLAYTIME_HOURS (*(u_char *)0x801F29BC)
#define CLOCK_HOURS    (*(u_char *)0x801F29C0)

/* Moves vertex `v` of both bobbing lists one step along channel `ch`'s
   direction, turning the direction round every `period` calls. */
void FieldBobVerts(int ch, int v, int period)
{
    if (g_bob_count[ch]++ == period) {
        g_bob_step[ch] ^= ~1;
        g_bob_count[ch] = 0;
    }
    g_bob_verts_a[v].vx += g_bob_step[ch];
    g_bob_verts_a[v].vy += g_bob_step[ch];
    g_bob_verts_b[v].vy += g_bob_step[ch];
    g_bob_verts_b[v].vz += g_bob_step[ch];
}

/* Nine primitives of 0x20 bytes, each with four brightness bytes a word
   apart: all four count down together and jump back to 0x4F from zero. */
void FieldFadePrims(u_char *prim)
{
    int i;

    for (i = 0; i < 9; i++) {
        if (prim[0] == 0) {
            prim[0] += 0x4F;
            prim[4] += 0x4F;
            prim[8] += 0x4F;
            prim[12] += 0x4F;
        } else {
            prim[0]--;
            prim[4]--;
            prim[8]--;
            prim[12]--;
        }
        prim += 0x20;
    }
}

/* Advances everything that counts time by `frames`: the field's own frame
   counter, the countdown (unless it is off or held) and the play clock, which
   stops just short of 100 hours.
 *
 * The frame counter's wrap is a ternary on the subtrahend: that is what
 * leaves the image's sixteen bytes of frame nothing reads, and what keeps the
 * sum in one register. Each hours carry reads the hours before it writes the
 * minutes back - written the other way round the load comes a store later. */
void FieldClockTick(int frames)
{
    u_char *h;

    g_field_frames = frames + g_field_frames;
    g_field_frames -= g_field_frames >= 60 ? 60 : 0;

    if (g_clock_on != 0 && g_clock_hold == 0 && g_clock_freeze == 0) {
        h = &CLOCK_HOURS;
        if ((g_clock_frame = frames + g_clock_frame) >= 60) {
            g_clock_frame -= 60;
            g_clock_sec++;
        }
        if (g_clock_sec >= 60) {
            g_clock_sec -= 60;
            g_clock_min++;
        }
        if (g_clock_min >= 60) {
            u_char hours = *h;
            g_clock_min -= 60;
            *h = hours + 1;
        }
    }

    if (g_playtime_hours == 99 && g_playtime_min == 59 && g_playtime_sec == 59 &&
        g_playtime_frame >= 57) {
        return;
    }
    h = &PLAYTIME_HOURS;
    g_playtime_frame = frames + g_playtime_frame;
    if (g_playtime_frame >= 60) {
        g_playtime_frame -= 60;
        g_playtime_sec++;
    }
    if (g_playtime_sec >= 60) {
        g_playtime_sec -= 60;
        g_playtime_min++;
    }
    if (g_playtime_min >= 60) {
        u_char hours = *h;
        g_playtime_min -= 60;
        *h = hours + 1;
    }
}

/* One frame of a door opening: each half moves along its axis and has its
   matrix rebuilt, until the frame count runs out. */
void FieldDoorSlide(void)
{
    GsCOORDINATE2 *c;

    if (g_door_state == 1) {
        if (g_door_frames != 0) {
            c = g_scene->objs[g_dng->door_obj[0]].coord2;
            c->coord.t[g_dng->door_axis[0]] += g_door_dx;
            g_scene->objs[g_dng->door_obj[0]].coord2->flg = 0;
            c = g_scene->objs[g_dng->door_obj[1]].coord2;
            c->coord.t[g_dng->door_axis[1]] += g_door_dy;
            g_scene->objs[g_dng->door_obj[1]].coord2->flg = 0;
            g_door_frames--;
        } else {
            g_door_state = 0;
        }
    }
}

/* Turns the clock face at scene object `obj` to the time left on the
   countdown, a full turn to six hours, and rebuilds the matrices of it and
   the object after it. Only for an object whose flags carry bit 12, and only
   when the low five bits are clear. */
void FieldClockHands(u_short flags, int obj, int col, int row)
{
    int left;

    if ((flags & 0x1000) && !(flags & 0x1F)) {
        left = g_clock_limit_hours[g_dng->map] * 3600 + g_clock_limit_min[g_dng->map] * 60 -
               ((u_char)g_clock_hours * 3600 + (u_char)g_clock_min * 60 + (u_char)g_clock_sec);
        g_scene->rots[obj].vz = 0x1000 - (left << 12) / 21600;
        CoordSetRot(&g_scene->rots[obj], &g_scene->coords[obj]);
        CoordSetRot(&g_scene->rots[obj + 1], &g_scene->coords[obj + 1]);
    }
}
