/* Persona 1 (JP) - the short-lived marks a blow or a cast puts on the field.
 *   BTLP only.
 *   0x800848D0 BtlSpawnStrike      0x80084A14 BtlSpawnCastCircle
 *   0x80084BFC BtlSpawnHitNumber   0x80084DC0 BtlSpawnMiss
 *   0x80084E10 BtlSpawnImpact
 *
 * Five spawners, each putting one record up at a position and handing it
 * back.
 *
 * BtlSpawnStrike unpacks one of the two strike sets the loader leaves behind
 * the backdrop - its TIM to the stage and its artwork to the effect slot -
 * binds the model asked for, and puts the record up with the spare graphics
 * slot's script table.
 *
 * BtlSpawnCastCircle stands two records on a grid cell, the second laid flat
 * under the first, out of four of the marker scripts. Side nought's cell is
 * ten rows further down the field, which is where the party stands.
 *
 * BtlSpawnHitNumber draws the amount itself. The record carries four cells of
 * its own and a frame pointing at them, the digits are written straight into
 * the cells from the right, and the row is then centred on however many digits
 * there turned out to be.
 *
 * BtlSpawnMiss puts up the 32-pixel word that sits beside the digits in the
 * same row of the font, with the kind a still number takes, and BtlSpawnImpact
 * a two-frame spark that loops.
 */
#include <decomp/types.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/fieldmarks.h>
#include <persona/btlp/object.h>
#include <persona/btlp/round.h>
#include <persona/btlp/spellfx.h>
#include <persona/btlp/text.h>

/* Where a strike set is unpacked to, and how its TIM is put up. */
#define STRIKE_TIM      ((u_char *)0x80140000)
#define STRIKE_GFX      ((u_char *)0x801D9400)
#define STRIKE_BIND     3
#define STRIKE_TIM_PAGE 0x1D
#define STRIKE_TIM_SLOT 0xE
#define STRIKE_MOTION   2
#define STRIKE_ATTR     (BTL_OBJ_ATTR_2000 | BTL_OBJ_NO_SHADOW)

/* The circle: which marker scripts its two records run, and the grid it is
   laid on - fifteen pixels a half column and twenty a row, from an origin
   four columns and seven rows in. */
#define CIRCLE_SCRIPT      12
#define CIRCLE_ATTR        1
#define CIRCLE_CD          0x1C
#define CIRCLE_CE          0x11
#define CIRCLE_FLAT_ATTR   (BTL_OBJ_ATTR_4000 | BTL_OBJ_TRACKING | 0x4)
#define CIRCLE_FLAT_SCALE  0x2000
#define CELL_W             15
#define CELL_X0            60
#define CELL_H             20
#define CELL_Y0            140
#define CIRCLE_PARTY_ROWS  10

/* The number: four cells eight pixels square out of the font row the miss
   word shares, blank until a digit is written in. */
#define NUMBER_CELLS  4
#define NUMBER_DIGITS 4
#define NUMBER_CD     0x1F
#define NUMBER_GLYPH  8
#define NUMBER_TOP    (-4)
#define NUMBER_U0     0x60
#define NUMBER_V      0x56
#define NUMBER_RISE   (-0x140000)
#define NUMBER_TIMER  0x14

#define IMPACT_CE    0x29
#define IMPACT_TIMER 0x3C

/* The template attribute every mark but the circle starts with. */
#define MARK_ATTR       0x200C0000
#define IMPACT_ATTR     0xC0000
#define IMPACT_FRAME    8
#define IMPACT_LOOP     0xFE00

extern void BtlUnpack(u_char *dst, const u_char *src);
extern int  BtlBindGfx(u_int kind, int index, u_char **image);
extern u_long *BtlUploadTim(u_long *tim, int page, int slot, int abr, int y,
                            int put);

/* The two templates filled in afresh before each allocation. */
BtlObjDef g_btl_strike_def = {0, 0};
BtlObjDef g_btl_cast_circle_def = {0, 0};

BtlObjDef g_btl_hit_number_def = {MARK_ATTR, 0};

BtlGfxCell g_btl_miss_cell = {-16, -4, 0x38, NUMBER_V, 0x20, NUMBER_GLYPH};
BtlGfxList g_btl_miss_frame = {1, &g_btl_miss_cell};
BtlSeqStep g_btl_miss_script = {(u_long)&g_btl_miss_frame, 1};
BtlObjDef  g_btl_miss_def = {MARK_ATTR, (const u_long **)&g_btl_miss_script};

BtlGfxCell g_btl_impact_cells[4] = {
    {-4, -5, 0x00, 0xD0, 8, 10},
    {-4, -5, 0x00, 0xDC, 8, 10},
    {-4, -5, 0x08, 0xD0, 8, 10},
    {-4, -5, 0x08, 0xDC, 8, 10},
};

BtlGfxList g_btl_impact_frames[4] = {
    {1, &g_btl_impact_cells[0]},
    {1, &g_btl_impact_cells[1]},
    {1, &g_btl_impact_cells[2]},
    {1, &g_btl_impact_cells[3]},
};

/* Two frames eight ticks each, and a step back to the first. */
BtlSeqStep g_btl_impact_script0[3] = {
    {(u_long)&g_btl_impact_frames[0], IMPACT_FRAME},
    {(u_long)&g_btl_impact_frames[1], IMPACT_FRAME},
    {(u_long)g_btl_impact_script0, IMPACT_LOOP},
};

BtlSeqStep g_btl_impact_script1[3] = {
    {(u_long)&g_btl_impact_frames[2], IMPACT_FRAME},
    {(u_long)&g_btl_impact_frames[3], IMPACT_FRAME},
    {(u_long)g_btl_impact_script1, IMPACT_LOOP},
};

BtlObjDef g_btl_impact_def = {IMPACT_ATTR,
                              (const u_long **)g_btl_impact_script0};

BtlObj *BtlSpawnStrike(int set, int model, const long *pos)
{
    u_char *tim;
    u_char *gfx;
    BtlObj *o;

    BtlCloseMessage(0);
    if (set == 0) {
        tim = g_btl_strike_tim0;
        gfx = g_btl_strike_gfx0;
    } else {
        tim = g_btl_strike_tim1;
        gfx = g_btl_strike_gfx1;
    }
    BtlUnpack(STRIKE_TIM, tim);
    BtlUnpack(STRIKE_GFX, gfx);
    g_btl_fx_gfx = STRIKE_GFX;
    BtlBindGfx(STRIKE_BIND, model, &g_btl_fx_gfx);
    BtlUploadTim((u_long *)STRIKE_TIM, STRIKE_TIM_PAGE, STRIKE_TIM_SLOT, 1, 0,
                 1);

    g_btl_strike_def.attr = 0;
    g_btl_strike_def.scripts = ((const u_long ***)g_btl_unused_gfx)[model];
    o = BtlObjAlloc(&g_btl_strike_def, FX_OBJ_GROUP, 0, FX_OBJ_DRAW, 0, pos,
                    FX_OBJ_CD, FX_OBJ_CE);
    o->attr |= STRIKE_ATTR;
    o->scripts = (const u_long **)g_btl_unused_gfx;
    o->kind = model;
    o->motion = STRIKE_MOTION;
    return o;
}

BtlObj *BtlSpawnCastCircle(int side, int col2, int row)
{
    long    pos[3];
    BtlObj *o;

    g_btl_cast_circle_def.attr = CIRCLE_ATTR;
    g_btl_cast_circle_def.scripts =
        (const u_long **)g_btl_marker_scripts[CIRCLE_SCRIPT];
    if (side == 0) {
        pos[0] = (col2 * CELL_W - CELL_X0) << 16;
        pos[1] = ((row + CIRCLE_PARTY_ROWS) * CELL_H - CELL_Y0) << 16;
        pos[2] = 0;
    } else {
        pos[0] = (col2 * CELL_W - CELL_X0) << 16;
        pos[1] = (row * CELL_H - CELL_Y0) << 16;
        pos[2] = 0;
    }

    o = BtlObjAlloc(&g_btl_cast_circle_def, FX_OBJ_GROUP, 0, FX_OBJ_DRAW, 0,
                    pos, CIRCLE_CD, CIRCLE_CE);
    o->kind = 0;
    o->col2 = col2;
    o->row = row;
    o->attr |= BTL_OBJ_TRACKING;
    o->next_script = (const u_long *)g_btl_marker_scripts[CIRCLE_SCRIPT + 2];

    g_btl_cast_circle_def.scripts =
        (const u_long **)g_btl_marker_scripts[CIRCLE_SCRIPT + 1];
    o->attached = BtlObjAlloc(&g_btl_cast_circle_def, FX_OBJ_GROUP, 0,
                              FX_OBJ_DRAW, 0, pos, CIRCLE_CD, CIRCLE_CE);
    o->attached->rot.vx = g_btl_cam_rot.vx;
    o->attached->rot.vz = g_btl_intro_dist;
    o->attached->scale_x = CIRCLE_FLAT_SCALE;
    o->attached->scale_y = CIRCLE_FLAT_SCALE;
    o->attached->attr |= CIRCLE_FLAT_ATTR;
    o->attached->kind = 0;
    o->attached->col2 = col2;
    o->attached->row = row;
    o->attached->next_script =
        (const u_long *)g_btl_marker_scripts[CIRCLE_SCRIPT + 3];
    return o;
}

BtlObj *BtlSpawnHitNumber(u_int value, const long *pos, int kind)
{
    BtlObj *o;
    int     i;
    int     digits;
    int     x;

    o = BtlObjAlloc(&g_btl_hit_number_def, 0, 0, FX_OBJ_DRAW, 0, pos,
                    NUMBER_CD, kind);
    /* The amount is kept on the record as well as drawn. */
    o->scale_to = value;
    o->kind = (kind == HIT_NUMBER_STILL) ? MARK_KIND_STILL : MARK_KIND_RISING;
    o->last = (u_long)&o->frame;
    o->frame.count = NUMBER_CELLS;
    o->frame.cells = o->cells;
    if (kind == HIT_NUMBER_STILL) {
        o->kind = MARK_KIND_STILL;
    } else {
        o->kind = MARK_KIND_RISING;
        o->shift = NUMBER_RISE;
        o->timer = NUMBER_TIMER;
    }

    for (i = 0; i < NUMBER_CELLS; i++) {
        o->cells[i].y = NUMBER_TOP;
        o->cells[i].u = 0;
        o->cells[i].v = 0;
        o->cells[i].w = NUMBER_GLYPH;
        o->cells[i].h = NUMBER_GLYPH;
    }

    digits = 1;
    o->cells[NUMBER_CELLS - 1].v = NUMBER_V;
    o->cells[NUMBER_CELLS - 1].u = value % 10 * NUMBER_GLYPH + NUMBER_U0;
    value /= 10;
    for (i = 0; i < NUMBER_DIGITS - 1; i++) {
        if (value == 0) {
            break;
        }
        digits++;
        o->cells[NUMBER_CELLS - 2 - i].v = NUMBER_V;
        o->cells[NUMBER_CELLS - 2 - i].u = value % 10 * NUMBER_GLYPH
                                           + NUMBER_U0;
        value /= 10;
    }

    /* The row's own x, stepped rather than worked out from the cell: the
       image keeps it as a counter of its own, and steps it after the cell. */
    i = 0;
    x = (digits - 8) * 4;
    do {
        o->cells[i].x = x;
        i++;
        x += NUMBER_GLYPH;
    } while (i < NUMBER_CELLS);
    return o;
}

BtlObj *BtlSpawnMiss(const long *pos)
{
    BtlObj *o;

    o = BtlObjAlloc(&g_btl_miss_def, 0, 0, FX_OBJ_DRAW, 0, pos, NUMBER_CD,
                    HIT_NUMBER_STILL);
    o->kind = MARK_KIND_STILL;
    return o;
}

BtlObj *BtlSpawnImpact(int alt, const long *pos)
{
    BtlObj *o;

    g_btl_impact_def.scripts = alt ? (const u_long **)g_btl_impact_script1
                                   : (const u_long **)g_btl_impact_script0;
    o = BtlObjAlloc(&g_btl_impact_def, 0, 0, FX_OBJ_DRAW, 0, pos, NUMBER_CD,
                    IMPACT_CE);
    o->kind = MARK_KIND_IMPACT;
    o->timer = IMPACT_TIMER;
    return o;
}
