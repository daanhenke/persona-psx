/* Persona 1 (JP) - the board the fight picks a spell off.  BTLP only.
 *   0x800AB85C BtlBuildSpellLines
 *
 * The spell half of BtlBuildItemLines, filling the same twelve lines of the
 * same board. There is nothing to search for: the menu keeps a spell id in
 * g_btl_spell_slot and the board is the twelve ids from there on, which is
 * why the caller steps that id by two rather than walking a list.
 *
 * A line is the spell's name, an end marker where the item board writes a
 * count, and the height of the two rows it is drawn as - zero for the last
 * two of the twelve, so the list holds twelve and the board shows ten, the
 * same way the item board does.
 *
 * The colour is the one thing the item board has no equivalent of: a spell
 * whose record in g_btl_spell_fx carries no start handler is drawn in the
 * grey the tactics page uses for an empty slot, and every other spell in the
 * live colour.
 */
#include <decomp/types.h>
#include <decomp/libc.h>
#include <persona/btlp/object.h>
#include <persona/common/spell.h>

/* Lines the list holds, and how many of them are drawn. */
#define SPELL_LINES 12
#define SPELL_ROWS  10

/* How tall a line that is drawn stands. */
#define SPELL_LINE_H 12

/* The two colours a line is drawn in: the spell does something, and it does
   not. The same pair the tactics page greys an empty row with. */
#define SPELL_CLUT_LIVE 0x20
#define SPELL_CLUT_NONE 0x23

/* What a spell does, indexed by the same id g_spell_data is. Record zero is
   the empty one, so the table is as long as the spell list plus its head.
   The three handlers are the effect's start, its step and its finish; `group`
   sorts the spell into one of five families. */
typedef struct {
    /* 0x0 */ void (*start)();
    /* 0x4 */ void (*step)();
    /* 0x8 */ void (*finish)();
    /* 0xC */ int  group;
} BtlSpellFx;                          /* 16 bytes */

extern BtlSpellFx g_btl_spell_fx[];

/* The three things each line is made of, and the rows they are drawn as -
   twelve for the board and twelve more directly behind them. */
extern u_char     g_btl_item_names[][10];
extern u_char     g_btl_item_counts[][3];
extern BtlGfxText g_btl_item_rows[];

void BtlBuildSpellLines(int spell)
{
    BtlGfxText *row;
    BtlGfxText *row2;
    u_char     *name;
    u_char     *count;
    int         i;

    name  = g_btl_item_names[0];
    count = g_btl_item_counts[0];
    row   = g_btl_item_rows;
    row2  = row + SPELL_LINES;
    i     = 0;
    do {
        memcpy(name, g_spell_data[spell].name, SPELL_NAME_CELLS);
        if (g_btl_spell_fx[spell].start == 0) {
            row->clut = SPELL_CLUT_NONE;
        } else {
            row->clut = SPELL_CLUT_LIVE;
        }
        *count = BTL_TEXT_END;
        if (i < SPELL_ROWS) {
            row->h = SPELL_LINE_H;
        } else {
            row->h = 0;
        }
        if (i < SPELL_ROWS) {
            row2->h = SPELL_LINE_H;
        } else {
            row2->h = 0;
        }
        name  += sizeof(g_btl_item_names[0]);
        count += sizeof(g_btl_item_counts[0]);
        row++;
        row2++;
        i++;
        spell++;
    } while (i < SPELL_LINES);
}
