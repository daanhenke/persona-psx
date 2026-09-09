/* Persona 1 (JP) - stat bars drawn straight from the Persona table.
 *
 *   ADV 0x8008DA44 ..
 *
 * The tail of the stat-bar unit. The routine between these and the ones in
 * statbars.c has not been worked out yet, so the overlays take it from asm
 * and these two are an object of their own.
 */
#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>
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

