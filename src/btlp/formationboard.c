/* Persona 1 (JP) - the board the eight stored formations are listed on.
 * BTLP only.
 *   0x800AA074 BtlOpenFormationBoard  0x800AA1B8 BtlCloseFormationBoard
 *
 * The player can keep eight grid layouts, and g_btl_formation_preset holds them
 * immediately in front of the live grid - eight blocks of twenty-five cells
 * against the one the fight is being fought on. This board is the list of them.
 *
 * Every row is written before the board goes up, because the list does not
 * change while it stands. A slot nobody has written to gets the empty name; one
 * that has been written to gets the same label as every other, with the slot
 * number after it, so the eight rows differ by one cell.
 *
 * What differs is the colour, and each kind of row decides it differently:
 *
 *   a stored layout is live when it places as many fighters as the party has
 *   and grey when it does not, which is the only check made - the cells
 *   themselves are never compared, only how many of them are occupied;
 *
 *   an empty slot is live only while the choice above the board is on its
 *   first row and grey otherwise, so whether nothing is a valid answer is
 *   settled before the board is opened rather than here.
 *
 * The two colours are the pair the rest of the overlay greys a dead row with -
 * the same ones BtlBuildSpellLines uses for a spell that does nothing.
 *
 * The line under the eight is written after the loop, out of the one place the
 * pointer has already reached, which is why it costs no arithmetic of its own.
 */
#include <decomp/types.h>
#include <decomp/libc.h>
#include <persona/btlp/board.h>
#include <persona/btlp/formation.h>
#include <persona/btlp/object.h>

/* Slots the player can store a layout in. */
#define PRESET_SLOTS 8

/* Cells a row is written with: the label, and the slot number behind it. */
#define PRESET_NAME_CELLS 8
#define PRESET_EMPTY_CELLS 6

/* The slot number is drawn from a bank that starts below the label's, so the
   number's cell is the slot less the distance between them. */
#define PRESET_SLOT_CELL 0x5A

/* Live and grey, the pair used wherever a row is drawn dead. */
#define PRESET_CLUT_LIVE 0x20
#define PRESET_CLUT_NONE 0x23

/* The rows, one per slot and one under them, and the text each is written
   with. The last line's label is four cells where a slot's is eight; the copy
   is the same size either way. */
extern BtlGfxText   g_btl_formation_rows[];
extern u_char       g_btl_formation_lines[][PRESET_NAME_CELLS + 1];
extern const u_char g_btl_preset_name[];
extern const u_char g_btl_preset_last_line[];
extern const u_char g_btl_name_empty[];

/* Which row the choice above the board is on. */
extern short g_btl_choice0_row;

/* The board itself, and what it is built from. */
extern BtlObj            *g_btl_formation_board;
extern const BtlBoardDef  g_btl_formation_board_defs[];
extern const long         g_btl_formation_board_pos[];

/* Whether a slot has ever been written to, and whether what is in it places
   the same number of fighters as the party. Both still assembly. */
extern int BtlFormationPresetEmpty(int slot);
extern int BtlFormationPresetFits(int slot);

void BtlOpenFormationBoard(void)
{
    u_char *line;
    u_char *clut;
    int     colour;
    int     i;

    line = g_btl_formation_lines[0];
    i    = 0;
    clut = &g_btl_formation_rows[0].clut;
    do {
        if (BtlFormationPresetEmpty(i) != 0) {
            memcpy(line, g_btl_name_empty, PRESET_EMPTY_CELLS);
            colour = PRESET_CLUT_NONE;
            if (g_btl_choice0_row == 0) {
                colour = PRESET_CLUT_LIVE;
            }
            *clut = colour;
        } else {
            memcpy(line, g_btl_preset_name, PRESET_NAME_CELLS);
            line[PRESET_NAME_CELLS] = i - PRESET_SLOT_CELL;
            *clut = PRESET_CLUT_LIVE;
            if (BtlFormationPresetFits(i) == 0) {
                *clut = PRESET_CLUT_NONE;
            }
        }
        i++;
        clut += sizeof(BtlGfxText);
        line += sizeof(g_btl_formation_lines[0]);
    } while (i < PRESET_SLOTS);
    memcpy(line, g_btl_preset_last_line, PRESET_NAME_CELLS);
    g_btl_formation_board =
        BtlBoardOpen(g_btl_formation_board_defs, g_btl_formation_board_pos);
}

void BtlCloseFormationBoard(void)
{
    BtlBoardShut(g_btl_formation_board);
}
