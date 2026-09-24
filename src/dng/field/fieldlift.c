/* Persona 1 (JP) - the lifts.  DNG only.
 *   0x8007360C FieldSetLiftDigits
 *   0x80073714 FieldRideLift
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <persona/dng/field.h>

/* The lift's floor indicator: seven sprites from LIFT_SPRITE on, each cut
   to a cell of the pattern for the number shown, and a sign sprite shown
   for the basement floors below LIFT_BASEMENT. */
#define LIFT_SPRITE   82
#define LIFT_DIGITS   7
#define LIFT_SIGN     89
#define LIFT_BASEMENT 10

void FieldSetLiftDigits(int n)
{
    int i;

    int k;

    for (i = 0; i < LIFT_DIGITS; i++) {
        k = g_lift_patterns[g_lift_pattern_of[n]][i];
        g_scene->sprites[LIFT_SPRITE + i].w = g_lift_cells[k].w;
        g_scene->sprites[LIFT_SPRITE + i].h = g_lift_cells[k].h;
        g_scene->sprites[LIFT_SPRITE + i].u = g_lift_cells[k].u;
        g_scene->sprites[LIFT_SPRITE + i].v = g_lift_cells[k].v;
    }
    if (n >= LIFT_BASEMENT) {
        g_scene->sprites[LIFT_SIGN].attribute &= 0x7FFFFFFF;
    } else {
        g_scene->sprites[LIFT_SIGN].attribute |= 0x80000000;
    }
}

/* Takes the lift the party stands in to button `button`: a jingle and a
   jolt, the indicator stepping a floor every thirty frames, a jolt the
   other way on arrival, and the floor it stops at loaded. */
/* 93.7%: the difference and the floors left swap registers, and the image
   reads g_dng before working out the floor it stops at, where here the
   read follows. The departure floor is its own array, a half-row into the
   lift table. */
#ifdef NON_MATCHING
void FieldRideLift(int button)
{
    int diff, left, dir, i;

    diff = g_lift_stops[g_scene->lift][button] - g_scene->lift_at;
    left = diff;
    if (left < 0) {
        left = -left;
    }
    dir = 1;
    if (diff < 0) {
        dir = -1;
    }
    FieldPlayJingle(0x19, 1, 1);
    FieldShake(dir);
    for (; left > 0; left--) {
        for (i = 0; i < 30; i++) {
            func_80065978();
        }
        g_scene->lift_at += dir;
    }
    FieldPlayJingle(0x1A, 1, 1);
    FieldShake(-dir);
    for (i = 0; i < 20; i++) {
        func_80065978();
    }
    g_dng->floor = g_lift_from[g_scene->lift][g_scene->lift_y * 2 + g_scene->lift_x];
    g_floor_info = (u_char *)(INDEX_BASE + g_index_info_tab[DNG_FLOOR]);
    g_floor_grid = (void *)(INDEX_BASE + g_index_grid_tab[DNG_FLOOR]);
    g_floor_objs = (u_char *)(PACK_BASE + g_pack_obj_tab[DNG_FLOOR]);
    g_floor_spots = (u_char *)(PACK_BASE + g_pack_spot_tab[DNG_FLOOR]);
    g_floor_events = (u_char *)(PACK_BASE + g_pack_event_tab[DNG_FLOOR]);
    FieldRebuildMap();
    func_8006F510(1);
    func_80070090(0);
    FieldLoadWallCluts();
}
#else
INCLUDE_ASM("dng/nonmatchings/field/fieldlift", FieldRideLift);
#endif
