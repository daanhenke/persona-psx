/* Persona 1 (JP) - putting the party's markers away and filling them in.
 *   BTLP only.
 *   0x800A833C BtlRetractMarkers   0x800A83B4 BtlBuildMarkers
 *
 * BtlRetractMarkers sends all six marker objects off together: no wait, the
 * first phase of the motion, and half their size to shrink toward.
 *
 * BtlBuildMarkers refills what each member's marker shows. For a member in the
 * slot it copies the name, the ailment's label and the equipped Persona's name
 * into the marker's own buffers, draws the level, hp and sp into their digit
 * cells, puts the name in the low colour at a quarter of hp or under, and
 * tints the two gauges.
 *
 * A member still fighting then has the name centred on the glyphs it really
 * has, the gauge glyphs back in the last two rows, and one small cell placed
 * for every member still fighting at that member's spot on the grid - a
 * different cell for the marker's own member. Anyone else has the marker
 * dimmed and every cell parked: a member who is down or has escaped shows
 * DYING or ESCAPE where the hp glyph goes, and an empty slot shows EMPTY with
 * its numbers blanked.
 */
#include <decomp/types.h>
#include <decomp/libc.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/board.h>
#include <persona/btlp/number.h>
#include <persona/btlp/object.h>
#include <persona/btlp/stats.h>

/* Retracting: all six objects, straight away, halving toward nothing. */
#define MARKER_OBJS    6
#define RETRACT_PHASE  1
#define RETRACT_SCALE  0x80
#define RETRACT_MOTION 4

/* The five text rows a member's marker is drawn from. */
#define MARKER_ROWS 5
#define ROW_NAME    0
#define ROW_HP      1
#define ROW_SP      2
#define ROW_HP_MARK 3
#define ROW_SP_MARK 4

#define NAME_CELLS    8
#define LABEL_CELLS   6
#define PERSONA_CELLS 10
#define LEVEL_DIGITS  2
#define GAUGE_DIGITS  3

/* Where the name stands once it is centred - each glyph it has pulls it half
   a glyph left - and where EMPTY, the hp glyph and the two state labels
   stand. */
#define NAME_X       12
#define NAME_EMPTY_X (-8)
#define MARK_X       (-20)
#define DYING_X      (-8)
#define ESCAPE_X     (-12)
#define DYING_CELLS  5
#define ESCAPE_CELLS 6

/* The two state labels are the tail of the first two entries of the test
   party's name table - entries no key reaches - and the image reaches them
   through it. */
#define LABEL_DYING  (g_btl_test_party_names[0] + 4)
#define LABEL_ESCAPE (g_btl_test_party_names[1] + 2)

/* Which cell of the grid picture stands for a member: the marker's own
   member, the others, and nobody. */
#define CELL_SELF   0
#define CELL_OTHER  8
#define CELL_ABSENT 0x10

#define MARKER_Y_STEP 4
#define MARKER_Y0     (-0xC)

/* How dim a marker that cannot act goes, and how fast. */
#define MARKER_DIM  0x40
#define MARKER_FADE 0xFF

extern BtlGfxText   g_btl_marker_rows[][MARKER_ROWS];
extern const u_char g_btl_marker_gauge_marks[];
extern const u_char g_btl_status_labels[][LABEL_CELLS];
extern u_char       g_btl_marker_status[][LABEL_CELLS];
extern u_char       g_btl_marker_persona_names[][PERSONA_CELLS];
extern u_char       g_btl_marker_level_cells[][LEVEL_DIGITS];
extern u_char       g_btl_marker_hp_cells[][GAUGE_DIGITS];
extern u_char       g_btl_marker_hp_max_cells[][GAUGE_DIGITS];
extern u_char       g_btl_marker_sp_cells[][GAUGE_DIGITS];
extern u_char       g_btl_marker_sp_max_cells[][GAUGE_DIGITS];

void BtlRetractMarkers(void)
{
    int i;

    for (i = 0; i < MARKER_OBJS; i++) {
        BtlObjSetTimer(g_btl_marker_obj[i], 0);
        BtlObjSetPhase(g_btl_marker_obj[i], RETRACT_PHASE);
        BtlObjSetScaleTo(g_btl_marker_obj[i], RETRACT_SCALE);
        BtlObjSetMotion(g_btl_marker_obj[i], RETRACT_MOTION);
    }
}

void BtlBuildMarkers(void)
{
    BtlGfxCell *mark;
    BtlActor   *a;
    BtlGfxText *row;
    int         i;
    int         j;
    int         n;
    int         persona;
    int         low;

    /* The low colour is a local of its own. It never gets a register, so
       reload puts it back at each use - which is what keeps the dying label's
       two colours from being folded into the escape label's. */
    low = GAUGE_LOW;
    i = 0;
    row = g_btl_marker_rows[0];
    a = g_btl_actors;
    for (; i < BTL_PARTY; i++, a++, row += MARKER_ROWS) {
        mark = &g_btl_member_marker[i * MARKER_ROW];
        if (a->c.key != 0) {
            memcpy(g_btl_member_names[i], a->c.name, NAME_CELLS);
            memcpy(g_btl_marker_status[i],
                   g_btl_status_labels[(signed char)a->c.status], LABEL_CELLS);
            BtlDrawNumber(g_btl_marker_level_cells[i], a->c.level,
                          LEVEL_DIGITS);
            BtlDrawNumberAlt(g_btl_marker_hp_cells[i], a->c.hp, GAUGE_DIGITS);
            BtlDrawNumberAlt(g_btl_marker_hp_max_cells[i], a->c.hp_max,
                             GAUGE_DIGITS);
            BtlDrawNumberAlt(g_btl_marker_sp_cells[i], a->c.sp, GAUGE_DIGITS);
            BtlDrawNumberAlt(g_btl_marker_sp_max_cells[i], a->c.sp_max,
                             GAUGE_DIGITS);
            persona = BtlActorPersona(i);
            if (persona >= 0 && a->c.blocked == 0) {
                memcpy(g_btl_marker_persona_names[i],
                       g_btl_personas[persona].name, PERSONA_CELLS);
            } else {
                g_btl_marker_persona_names[i][0] = GLYPH_END;
            }
            if (a->c.hp <= a->c.hp_max / GAUGE_LOW_AT) {
                row[ROW_NAME].clut = low;
            }
            BtlSetGaugeColour(&a->c, (u_char *)&row[ROW_HP],
                              (u_char *)&row[ROW_SP]);
        }

        if (g_btl_actors[i].c.key != 0
            && (signed char)g_btl_actors[i].c.status != BTL_STATUS_DOWN
            && (g_btl_actors[i].flags & BTL_ACTOR_OUT) == 0) {
            /* The glyphs are counted beside the walk rather than by it: the
               walk's own counter is what gcc turns into an address compare. */
            n = 0;
            for (j = 0; j < NAME_CELLS; j++) {
                if (g_btl_actors[i].c.name[j] == GLYPH_END) {
                    break;
                }
                n++;
            }
            /* The name's colour straight after its position: that is where
               the image loads the colour, well ahead of the stores that use
               it. */
            row[ROW_NAME].x = NAME_X - n * (BTL_FONT_W / 2);
            row[ROW_NAME].clut = GAUGE_OK;
            row[ROW_HP_MARK].x = MARK_X;
            row[ROW_HP_MARK].text = g_btl_marker_gauge_marks;
            row[ROW_HP_MARK].count = 1;
            row[ROW_HP_MARK].clut = GAUGE_OK;
            row[ROW_HP].h = BTL_FONT_H;
            row[ROW_SP].h = BTL_FONT_H;
            row[ROW_HP_MARK].h = BTL_FONT_H;
            row[ROW_SP_MARK].h = BTL_FONT_H;
            for (j = 0; j < BTL_PARTY; j++, mark++) {
                if (g_btl_actors[j].c.key != 0
                    && (signed char)g_btl_actors[j].c.status != BTL_STATUS_DOWN
                    && (g_btl_actors[j].flags & BTL_ACTOR_OUT) == 0) {
                    mark->x = g_btl_actors[j].obj->col2 / 2 * MARKER_X_STEP
                              + MARKER_X_FAR;
                    mark->y = g_btl_actors[j].obj->row * MARKER_Y_STEP
                              + MARKER_Y0;
                    if (i != j) {
                        mark->u = CELL_OTHER;
                    } else {
                        mark->u = CELL_SELF;
                    }
                } else {
                    mark->u = CELL_ABSENT;
                }
            }
        } else {
            BtlObjLast(g_btl_marker_obj[i])->rgb_to[0] = MARKER_DIM;
            BtlObjLast(g_btl_marker_obj[i])->rgb_to[1] = MARKER_DIM;
            BtlObjLast(g_btl_marker_obj[i])->rgb_to[2] = MARKER_DIM;
            BtlObjSetFade(g_btl_marker_obj[i], MARKER_FADE);
            for (j = 0; j < BTL_PARTY; j++, mark++) {
                mark->u = CELL_ABSENT;
            }
            if (a->c.key != 0) {
                if ((a->flags & BTL_ACTOR_OUT) != 0) {
                    row[ROW_HP_MARK].text = LABEL_ESCAPE;
                    row[ROW_HP_MARK].x = ESCAPE_X;
                    row[ROW_NAME].clut = GAUGE_OK;
                    row[ROW_HP_MARK].count = ESCAPE_CELLS;
                    row[ROW_HP_MARK].clut = GAUGE_OK;
                } else {
                    row[ROW_NAME].clut = low;
                    row[ROW_HP_MARK].text = LABEL_DYING;
                    row[ROW_HP_MARK].x = DYING_X;
                    row[ROW_HP_MARK].count = DYING_CELLS;
                    row[ROW_HP_MARK].clut = low;
                }
                row[ROW_HP].h = 0;
                row[ROW_SP].h = 0;
                row[ROW_SP_MARK].h = 0;
            } else {
                memcpy(g_btl_member_names[i], g_btl_name_empty, NAME_CELLS);
                row[ROW_NAME].x = NAME_EMPTY_X;
                row[ROW_HP_MARK].h = 0;
                row[ROW_SP_MARK].h = 0;
                g_btl_marker_level_cells[i][0] = GLYPH_END;
                g_btl_marker_hp_cells[i][0] = GLYPH_END;
                g_btl_marker_hp_max_cells[i][0] = GLYPH_END;
                g_btl_marker_sp_cells[i][0] = GLYPH_END;
                g_btl_marker_sp_max_cells[i][0] = GLYPH_END;
            }
        }
    }
}
