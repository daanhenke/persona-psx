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
