#ifndef PERSONA_ADV_MONEYBOX_H
#define PERSONA_ADV_MONEYBOX_H

/* Persona 1 (JP) - the money box.  ADV only.
 *
 * Background layer 5, put up by BgBoxShow: the party's money on its second
 * row and the play-time clock DrawStatusHud keeps on its third.
 */
#include <decomp/types.h>

/* Its cells, twelve across and four down. */
#define BOX_CELLS_W 0xC
#define BOX_CELLS_H 4

extern short g_panel_cells[BOX_CELLS_W * BOX_CELLS_H];

void BgBoxShow(void);

#endif
