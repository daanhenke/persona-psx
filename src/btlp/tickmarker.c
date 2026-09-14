/* Persona 1 (JP) - one frame for a record of the marker group.  BTLP only.
 *   0x8008AC10 BtlTickMarker
 *
 * g_btl_obj_tick's entry for group 1, which BtlTickObjects calls on every
 * record there.
 *
 * The placement grid's tail record also runs the grid's colour cycle. Each of
 * the grid's four lines walks its six colour channels toward a target a step
 * at a time, and once the first line has arrived the targets move one line
 * along, the first line's going round to the last.
 *
 * A picked record is then coloured: washed to white, left to BtlPulsePicked,
 * or held at half grey while the member it stands for can act and a quarter of
 * it otherwise. A tracking record takes the colour its member's marker is drawn
 * in. Last, the record's motion runs out of the group's table, or its phase is
 * put back to nought where the table has nothing.
 */
#include <decomp/types.h>
#include <decomp/libc.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/object.h>

/* The grid: the kind its tail record carries, how many lines it has, and how
   far a channel walks in a frame. */
#define GRID_TAIL_KIND 0xFF
#define GRID_LINES     4
#define GRID_FADE      0x30

/* Three bits a picked record can carry, tested in this order: washed to white,
   singled out and pulsing, and otherwise coloured by its member. */
#define MARKER_FRAME_BIT 0x800000
#define MARKER_WHITE     0x2000000
#define MARKER_SINGLED   0x1000000

#define MARKER_WHITE_RGB 0xFF
#define MARKER_LIT       0x80
#define MARKER_DIM       0x40
#define MARKER_FADE      0xFF

extern BtlGfxLine  g_btl_grid_lines[];
extern short       g_btl_grid_hold[];

extern void (*g_btl_obj_motion[])(BtlObj *o);

void BtlTickMarker(BtlObj *o)
{
    BtlGfxLine *line;
    BtlObj     *mark;
    int         i;
    int         j;

    if (g_btl_grid_tail != 0 && o->kind == GRID_TAIL_KIND) {
        line = g_btl_grid_lines;
        if (g_btl_grid_lines[0].rgb[0] == g_btl_grid_lines[0].rgb_to[0]
            && g_btl_grid_lines[0].rgb[1] == g_btl_grid_lines[0].rgb_to[1]
            && g_btl_grid_lines[0].rgb[2] == g_btl_grid_lines[0].rgb_to[2]) {
            memcpy(g_btl_grid_hold, g_btl_grid_lines[0].rgb_to,
                   sizeof(line->rgb_to));
            for (j = 0; j < GRID_LINES - 1; j++) {
                memcpy(line[j].rgb_to, line[j + 1].rgb_to,
                       sizeof(line->rgb_to));
            }
            memcpy(line[GRID_LINES - 1].rgb_to, g_btl_grid_hold,
                   sizeof(line->rgb_to));
        }
        for (i = 0; i < GRID_LINES; i++, line++) {
            BtlApproach(&line->rgb[0], &line->rgb_to[0], GRID_FADE);
            BtlApproach(&line->rgb[1], &line->rgb_to[1], GRID_FADE);
            BtlApproach(&line->rgb[2], &line->rgb_to[2], GRID_FADE);
            BtlApproach(&line->rgb[3], &line->rgb_to[3], GRID_FADE);
            BtlApproach(&line->rgb[4], &line->rgb_to[4], GRID_FADE);
            BtlApproach(&line->rgb[5], &line->rgb_to[5], GRID_FADE);
        }
    }

    if ((o->attr & MARKER_FRAME_BIT) != 0) {
        if ((o->attr & MARKER_WHITE) != 0) {
            o->rgb_to[0] = MARKER_WHITE_RGB;
            o->rgb_to[1] = MARKER_WHITE_RGB;
            o->rgb_to[2] = MARKER_WHITE_RGB;
            o->fade = MARKER_FADE;
        } else if ((o->attr & MARKER_SINGLED) != 0) {
            BtlPulsePicked(o);
        } else {
            if (g_btl_actors[o->mark_num].c.key != 0
                && (signed char)g_btl_actors[o->mark_num].c.status
                       != BTL_STATUS_DOWN
                && (g_btl_actors[o->mark_num].flags & BTL_ACTOR_OUT) == 0) {
                BtlObjSetRgb(o, MARKER_LIT, MARKER_LIT, MARKER_LIT);
            } else {
                BtlObjSetRgb(o, MARKER_DIM, MARKER_DIM, MARKER_DIM);
            }
            BtlObjSetFade(o, MARKER_FADE);
        }
    }

    if ((o->attr & BTL_OBJ_TRACKING) != 0) {
        mark = g_btl_marker_obj[o->mark_num];
        o->rgb_to[0] = mark->rgb[0];
        o->rgb_to[1] = mark->rgb[1];
        o->rgb_to[2] = mark->rgb[2];
        o->fade = MARKER_FADE;
    }

    if (g_btl_obj_motion[o->motion] != 0) {
        g_btl_obj_motion[o->motion](o);
    } else {
        o->phase = 0;
    }
}
