/* Persona 1 (JP) - the stock board the Persona change is picked on.
 * BTLP only.
 *   0x800AAF88 BtlOpenStockBoard  0x800AB2E0 BtlCloseStockBoard
 *
 * Everything the board shows is written before it goes up, the same way the
 * Persona board is, and into the same spell lines and SP cells. Its first line
 * is the equipped Persona's name and the two below it the member's other
 * stock Personas, packed to the top; the spell lines are the equipped
 * Persona's spells, packed the same way, each line's first cell set to the
 * end marker before anything is copied. Then the member's own name, its level
 * and the byte beside it, and its SP against its maximum.
 */
#include <decomp/types.h>
#include <decomp/libc.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/board.h>
#include <persona/btlp/number.h>
#include <persona/btlp/object.h>
#include <persona/btlp/round.h>
#include <persona/btlp/stats.h>
#include <persona/common/spell.h>

/* A line is ten cells and the end marker after them; the board has three
   Persona lines, and a member's stock list three entries. */
#define STOCK_LINE_CELLS 11
#define STOCK_NAME_CELLS 10
#define STOCK_LINES      3
#define STOCK_ENTRIES    3
#define STOCK_EMPTY      0xFF

/* How wide each number is drawn. */
#define STOCK_LEVEL_W 2
#define STOCK_SP_W    3

extern BtlObj            *g_btl_stock_board;
extern const BtlBoardDef  g_btl_stock_board_defs[];
extern const long         g_btl_stock_board_pos[];
extern u_char             g_btl_stock_lines[STOCK_LINES][STOCK_LINE_CELLS];
extern u_char             g_btl_stock_member_name[];
extern u_char             g_btl_stock_level_cells[];
extern u_char             g_btl_stock_unk56_cells[];

void BtlOpenStockBoard(void)
{
    BtlStats *p;
    int       n;
    int       equipped;
    int       id;
    int       i;

    equipped = BtlActorPersona(g_btl_actor_turn);
    g_btl_stock_lines[1][0] = BTL_TEXT_END;
    g_btl_stock_lines[2][0] = BTL_TEXT_END;
    p = &g_btl_personas[equipped];
    memcpy(g_btl_stock_lines[0], p->unk1F, STOCK_NAME_CELLS);
    /* Both lists are packed by a count of the lines written, indexed rather
       than walked: gcc turns the index into the pointer the image steps, and
       only then does the pointer's start come after the loop's constants. */
    n = 1;
    for (i = 0; i < STOCK_ENTRIES; i++) {
        id = g_btl_actors[g_btl_actor_turn].c.list[i];
        if (id != STOCK_EMPTY && id != equipped) {
            memcpy(g_btl_stock_lines[n], g_btl_personas[id].unk1F,
                   STOCK_NAME_CELLS);
            n++;
        }
    }
    n = 0;
    for (i = 0; i < BTL_STATS_SPELLS; i++) {
        g_btl_persona_spell_lines[i][0] = BTL_TEXT_END;
        if (p->spell[i] != 0) {
            memcpy(g_btl_persona_spell_lines[n], g_spell_data[p->spell[i]].name,
                   SPELL_NAME_CELLS);
            n++;
        }
    }
    memcpy(g_btl_stock_member_name, g_btl_actors[g_btl_actor_turn].c.name,
           STOCK_NAME_CELLS);
    BtlDrawNumber(g_btl_stock_level_cells,
                  g_btl_actors[g_btl_actor_turn].c.level, STOCK_LEVEL_W);
    BtlDrawNumber(g_btl_stock_unk56_cells,
                  g_btl_actors[g_btl_actor_turn].c.unk56, STOCK_LEVEL_W);
    BtlDrawNumberAlt(g_btl_persona_sp_cells,
                     g_btl_actors[g_btl_actor_turn].c.sp, STOCK_SP_W);
    BtlDrawNumberAlt(g_btl_persona_sp_max_cells,
                     g_btl_actors[g_btl_actor_turn].c.sp_max, STOCK_SP_W);
    g_btl_stock_board = BtlBoardOpen(g_btl_stock_board_defs,
                                     g_btl_stock_board_pos);
}

void BtlCloseStockBoard(void)
{
    BtlBoardShut(g_btl_stock_board);
}
