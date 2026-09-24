/* Persona 1 (JP) - a button layout drawn out on the controller page.
 *
 *   ADV @ 0x8008BD20   DNG @ 0x8008FF48
 *
 * Twenty entries in two columns of ten, each the name of the action a button
 * is given under the layout picked. The layout table holds an action index
 * per entry, and index 0 - no action - draws its name from the dimmed glyph
 * bank so the unused buttons read as greyed out.
 *
 * S2D builds this same source against a layer 0x20000 higher, which is what
 * WORK_BIAS says.
 */
#include <decomp/types.h>
#include <persona/common/pad.h>
#include <persona/common/tilemap.h>

/* The two columns, cleared first. */
#define COLUMN_ROWS    10
#define COLUMN_PITCH   14
#define LAYOUT_ROW     6
#define LAYOUT_COL     6
#define NAME_W         13

#define GLYPH_BANK_DIM 0xD7

void PadDrawLayout(u_char layout)
{
    u_char i;
    u_char action;

    TileMapFillRect(&g_tilemap1[LAYOUT_ROW * MAP_W + LAYOUT_COL], 0, NAME_W,
                    COLUMN_ROWS, MAP_W);
    TileMapFillRect(&g_tilemap1[LAYOUT_ROW * MAP_W + LAYOUT_COL + COLUMN_PITCH],
                    0, NAME_W, COLUMN_ROWS, MAP_W);
    for (i = 0; i < PAD_LAYOUT_ENTRIES; i++) {
        action = g_pad_layout_actions[layout * PAD_LAYOUT_ENTRIES + i];
        TileMapWriteRow(g_pad_action_names[action],
                        g_tilemap1 + (i % COLUMN_ROWS + LAYOUT_ROW) * MAP_W
                            + (i / COLUMN_ROWS) * COLUMN_PITCH + LAYOUT_COL,
                        action == 0 ? GLYPH_BANK_DIM : 0, NAME_W);
    }
}
