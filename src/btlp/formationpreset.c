/* Persona 1 (JP) - two questions about a stored formation layout.
 * BTLP only.
 *   0x800C5CC4 BtlFormationPresetEmpty  0x800C5D10 BtlFormationPresetFits
 *
 * The formation board asks both of every one of the eight layouts in
 * g_btl_formation_preset as it fills its rows: whether the slot has never been
 * written to - every cell still empty - and, if it has, whether it places as
 * many fighters as the live grid does. A layout with a different head count is
 * drawn greyed out, since it cannot be stood on the party as it is.
 *
 * Both reach a layout through pointer arithmetic on the table, (table +
 * slot)->cell[i]. Indexing the table, walking a pointer to the row, or
 * indexing the cells as one long run all build the row's address ahead of the
 * constant the loop compares against, where the image has it after.
 */
#include <decomp/types.h>
#include <persona/btlp/formation.h>

int BtlFormationPresetEmpty(int slot)
{
    int i;

    i = 0;
    do {
        if ((g_btl_formation_preset + slot)->cell[i] != CELL_EMPTY) {
            return 0;
        }
        i++;
    } while (i < GRID_CELLS);
    return 1;
}

int BtlFormationPresetFits(int slot)
{
    int i;
    int live;
    int stored;

    i = 0;
    live = 0;
    stored = 0;
    do {
        if ((g_btl_formation_preset + slot)->cell[i] != CELL_EMPTY) {
            stored++;
        }
        if (g_btl_formation[i] != CELL_EMPTY) {
            live++;
        }
        i++;
    } while (i < GRID_CELLS);
    return live == stored;
}
