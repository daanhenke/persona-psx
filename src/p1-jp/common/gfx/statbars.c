/* Persona 1 (JP) - the stat bars on the status screen.
 *
 * Compiled into three overlays rather than called across the boundary:
 *                           DNG         ADV         S2D
 *   DrawCharStatBars        0x800914F0  0x8008D420  0x80081A04
 *   DrawPersonaStatBar      0x800918D4  0x8008D804  0x80081DE8
 *   DrawPersonaStatBars     0x80091878  0x8008D7A8  0x80081D8C
 *   DrawPersonaDataStatBar  0x80091B78  0x8008DAA8  0x8008208C
 *   DrawPersonaDataStatBars 0x80091B14  0x8008DA44  0x80082028
 *
 * A bar is a run of cells four units wide, so a value splits into whole cells
 * and a remainder that picks a partly filled glyph out of the strip at v 0x84:
 * u 0 is a full cell and u 8, 0x10, 0x18 are one, two and three quarters. A
 * character's bar is drawn in two colours - stat_base solid with stat above it
 * - which is why DrawCharStatBar is a separate, longer routine.
 */
#include <types.h>
#include <libgs.h>
#include <persona/common/char.h>
#include <persona/common/persona.h>

/* One cell run per bar, 26 cells each, of which a bar uses 25. */
extern GsCELL *g_stat_bar_cells[];

#define BAR_CELLS 25
#define BAR_V     0x84
#define BAR_UNITS 4
#define BAR_W     8
#define STAT_MAX  99

extern void CellsClear(GsCELL *dst, u_char count);
extern void DrawCharStatBar(Char *rec, u_char stat);

void DrawCharStatBars(Char *rec)
{
    DrawCharStatBar(rec, 0);
    DrawCharStatBar(rec, 1);
    DrawCharStatBar(rec, 2);
    DrawCharStatBar(rec, 3);
    DrawCharStatBar(rec, 4);
}

/* stat_base is drawn solid and the growth on top of it in a second colour, so
   the cell where the two meet needs a glyph showing both. The strip holds one
   for every split that can occur: u 8, 0x10 and 0x18 are one, two and three
   quarters of base alone, 0x20..0x30 are one quarter of base with one to three
   of growth, 0x38/0x40 two quarters of base with one or two, 0x48 three with
   one, and 0x58..0x70 one to four quarters of growth alone. 0x50 is a whole
   cell of growth.

   Each arm writes the cell itself rather than falling out to a shared store -
   that is what the original does, and gcc merges the stores back together. */
void DrawCharStatBar(Char *rec, u_char stat)
{
    GsCELL *cell;
    int     value;
    int     base;
    int     grow;
    int     i;

    cell = g_stat_bar_cells[stat];
    CellsClear(cell, BAR_CELLS);
    switch (stat) {
    case 0:
        value = rec->stat[0];
        base = rec->stat_base[0];
        break;
    case 1:
        value = rec->stat[1];
        base = rec->stat_base[1];
        break;
    case 2:
        value = rec->stat[2];
        base = rec->stat_base[2];
        break;
    case 3:
        value = rec->stat[3];
        base = rec->stat_base[3];
        break;
    case 4:
        value = rec->stat[4];
        base = rec->stat_base[4];
        break;
    }
    if (value > STAT_MAX) {
        value = STAT_MAX;
    }
    if (base > STAT_MAX) {
        base = STAT_MAX;
    }
    grow = value - base;
    for (i = 0; i < base / BAR_UNITS; i++) {
        cell->u = 0;
        cell->v = BAR_V;
        cell++;
    }
    /* The cell straddling the two colours. If the growth ends inside it the
       bar is finished here; otherwise draw the full split and carry on. */
    switch (base % BAR_UNITS) {
    case 0:
        switch (grow) {
        case 0:
            return;
        case 1:
            cell->u = 0x58;
            cell->v = BAR_V;
            return;
        case 2:
            cell->u = 0x60;
            cell->v = BAR_V;
            return;
        case 3:
            cell->u = 0x68;
            cell->v = BAR_V;
            return;
        case 4:
            cell->u = 0x70;
            cell->v = BAR_V;
            return;
        }
        grow -= 4;
        cell->u = 0x70;
        cell->v = BAR_V;
        cell++;
        break;
    case 1:
        switch (grow) {
        case 0:
            cell->u = 0x08;
            cell->v = BAR_V;
            return;
        case 1:
            cell->u = 0x20;
            cell->v = BAR_V;
            return;
        case 2:
            cell->u = 0x28;
            cell->v = BAR_V;
            return;
        case 3:
            cell->u = 0x30;
            cell->v = BAR_V;
            return;
        }
        grow -= 3;
        cell->u = 0x30;
        cell->v = BAR_V;
        cell++;
        break;
    case 2:
        switch (grow) {
        case 0:
            cell->u = 0x10;
            cell->v = BAR_V;
            return;
        case 1:
            cell->u = 0x38;
            cell->v = BAR_V;
            return;
        case 2:
            cell->u = 0x40;
            cell->v = BAR_V;
            return;
        }
        grow -= 2;
        cell->u = 0x40;
        cell->v = BAR_V;
        cell++;
        break;
    case 3:
        switch (grow) {
        case 0:
            cell->u = 0x18;
            cell->v = BAR_V;
            return;
        case 1:
            cell->u = 0x48;
            cell->v = BAR_V;
            return;
        }
        grow -= 1;
        cell->u = 0x48;
        cell->v = BAR_V;
        cell++;
        break;
    }
    for (i = 0; i < grow / BAR_UNITS; i++) {
        cell->u = 0x50;
        cell->v = BAR_V;
        cell++;
    }
    switch (grow % BAR_UNITS) {
    case 1:
        cell->u = 0x58;
        cell->v = BAR_V;
        return;
    case 2:
        cell->u = 0x60;
        cell->v = BAR_V;
        return;
    case 3:
        cell->u = 0x68;
        cell->v = BAR_V;
        return;
    }
}

/* A Persona's bar has no base/growth split, so it is one colour throughout. */
void DrawPersonaStatBar(Persona *p, u_char stat)
{
    GsCELL *cell;
    int     value;
    int     i;

    cell = g_stat_bar_cells[stat];
    CellsClear(cell, BAR_CELLS);
    switch (stat) {
    case 0:
        value = p->stat[0];
        break;
    case 1:
        value = p->stat[1];
        break;
    case 2:
        value = p->stat[2];
        break;
    case 3:
        value = p->stat[3];
        break;
    case 4:
        value = p->stat[4];
        break;
    }
    if (value > STAT_MAX) {
        value = STAT_MAX;
    }
    for (i = 0; i < value / BAR_UNITS; i++) {
        cell->u = 0;
        cell->v = BAR_V;
        cell++;
    }
    if (value % BAR_UNITS != 0) {
        cell->u = (value % BAR_UNITS) * BAR_W;
        cell->v = BAR_V;
    }
}

void DrawPersonaStatBars(Persona *p)
{
    DrawPersonaStatBar(p, 0);
    DrawPersonaStatBar(p, 1);
    DrawPersonaStatBar(p, 2);
    DrawPersonaStatBar(p, 3);
    DrawPersonaStatBar(p, 4);
}

/* The same bar, read straight out of the reference table. */
void DrawPersonaDataStatBar(short id, u_char stat)
{
    PersonaData *data;
    GsCELL      *cell;
    int          value;
    int          i;

    data = &g_persona_data[id];
    cell = g_stat_bar_cells[stat];
    CellsClear(cell, BAR_CELLS);
    switch (stat) {
    case 0:
        value = data->stat[0];
        break;
    case 1:
        value = data->stat[1];
        break;
    case 2:
        value = data->stat[2];
        break;
    case 3:
        value = data->stat[3];
        break;
    case 4:
        value = data->stat[4];
        break;
    }
    if (value > STAT_MAX) {
        value = STAT_MAX;
    }
    for (i = 0; i < value / BAR_UNITS; i++) {
        cell->u = 0;
        cell->v = BAR_V;
        cell++;
    }
    if (value % BAR_UNITS != 0) {
        cell->u = (value % BAR_UNITS) * BAR_W;
        cell->v = BAR_V;
    }
}

/* The stock list shows a Persona out of the reference table before it belongs
   to anybody, so its bars come from there rather than from a live record. */
void DrawPersonaDataStatBars(short id)
{
    DrawPersonaDataStatBar(id, 0);
    DrawPersonaDataStatBar(id, 1);
    DrawPersonaDataStatBar(id, 2);
    DrawPersonaDataStatBar(id, 3);
    DrawPersonaDataStatBar(id, 4);
}
