/* Persona 1 (JP) - the board the acting fighter's Persona is shown on.
 * BTLP only.
 *   0x800AACB0 BtlOpenPersonaBoard  0x800AAF60 BtlClosePersonaBoard
 *
 * Everything the board shows is written before it goes up, the way the stored
 * formations are: the Persona's ten name cells, its level, the number beside
 * that, the fighter's SP against its maximum, and the seven spell slots.
 *
 * The seven rows are the only part that varies. A row is the spell's name out
 * of g_spell_data and a colour, and the colour is live unless the slot holds a
 * spell whose kind has the top bit clear - so an empty slot is drawn live
 * rather than grey, which is the opposite of what the spell board does with a
 * spell that has no effect.
 *
 * The row's first cell is set to the end marker before the name is copied over
 * it, so it is written and then lost every time round. The image does it.
 *
 * BtlOpenPersonaBoard is 98.71% and behind INCLUDE_ASM. The image's four
 * induction variables - one per occurrence of each array - are two counters
 * stepped together: loop.c merges givs of one counter with the same step,
 * never givs of two. j indexes the live clut and the line's end marker and
 * is the one tested; i indexes the grey clut and the name copy. What is left
 * is where the grey clut's step lands against the name's reload of
 * p->spell[i] at the bottom of the loop.
 *
 * Only the SP gauge is coloured, not an HP one: the call is handed a null for
 * the first bar, and the board has no second gauge to put one in.
 *
 * The number drawn beside the level is the record's +0x29, three cells wide.
 * It comes from PersonaDef +0x12 and is a constant of the Persona - in every
 * definition it is smaller than the level and it is never read arithmetically
 * anywhere, so what it counts is not worked out and its cells keep the address
 * splat gave them.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <decomp/libc.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/board.h>
#include <persona/btlp/number.h>
#include <persona/btlp/object.h>
#include <persona/btlp/round.h>
#include <persona/btlp/stats.h>
#include <persona/common/spell.h>

/* Cells the Persona's name takes. */
#define PERSONA_NAME_CELLS 10

/* How wide each number is drawn. Both SP figures cap at 999. */
#define PERSONA_LEVEL_W  2
#define PERSONA_NUMBER_W 3
#define PERSONA_SP_W     3

/* Where the board's fifteen cells are spoken for: the SP gauge, and the seven
   rows the spell slots are drawn as. */
#define PERSONA_SP_CELL    6
#define PERSONA_SPELL_CELL 8

/* Live and grey, the pair used wherever a row is drawn dead. */
#define PERSONA_CLUT_LIVE 0x20
#define PERSONA_CLUT_NONE 0x23

/* The bit of a spell's kind that keeps its row live. */
#define SPELL_KIND_LISTED 0x80

/* Where the board stands. */
#define BOARD_X 0xA00000
#define BOARD_Y 0xC40000

/* The cells the board is drawn from, and the text written into them. */
extern BtlGfxText g_btl_persona_cells[];
extern u_char     g_btl_persona_name[];
extern u_char     g_btl_persona_level_cells[];
extern u_char     D_800F5B3C[];

/* The board itself and what it is built from. */
extern BtlObj            *g_btl_persona_board;
extern const BtlBoardDef  g_btl_persona_board_defs[];

/* Still assembly: what walks a gauge's cells to the colour the value deserves.
   The first bar is not drawn here, so it is handed nothing. */

#ifdef NON_MATCHING
void BtlOpenPersonaBoard(void)
{
    BtlStats *p;
    long      pos[3];
    int       spell;
    int       i;
    int       j;

    p = &g_btl_personas[BtlActorPersona(g_btl_actor_turn)];
    memcpy(g_btl_persona_name, p->name, PERSONA_NAME_CELLS);
    BtlDrawNumber(g_btl_persona_level_cells, p->level, PERSONA_LEVEL_W);
    BtlDrawNumber(D_800F5B3C, p->sp_cost, PERSONA_NUMBER_W);
    BtlDrawNumberAlt(g_btl_persona_sp_cells,
                     g_btl_actors[g_btl_actor_turn].c.sp, PERSONA_SP_W);
    BtlDrawNumberAlt(g_btl_persona_sp_max_cells,
                     g_btl_actors[g_btl_actor_turn].c.sp_max, PERSONA_SP_W);
    i = 0;
    j = 0;
    do {
        g_btl_persona_cells[PERSONA_SPELL_CELL + j].clut = PERSONA_CLUT_LIVE;
        g_btl_persona_spell_lines[j][0] = BTL_TEXT_END;
        spell = p->spell[i];
        if (spell != 0 && (g_spell_data[spell].kind & SPELL_KIND_LISTED) == 0) {
            g_btl_persona_cells[PERSONA_SPELL_CELL + i].clut = PERSONA_CLUT_NONE;
        }
        memcpy(g_btl_persona_spell_lines[i], g_spell_data[p->spell[i]].name,
               SPELL_NAME_CELLS);
        i++;
        j++;
    } while (j < BTL_STATS_SPELLS);
    BtlSetGaugeColour(&g_btl_actors[g_btl_actor_turn].c, 0,
                      (u_char *)&g_btl_persona_cells[PERSONA_SP_CELL]);
    pos[0] = BOARD_X;
    pos[1] = BOARD_Y;
    pos[2] = 0;
    g_btl_persona_board = BtlBoardOpen(g_btl_persona_board_defs, pos);
}
#else
INCLUDE_ASM("btlp/nonmatchings/personaboard", BtlOpenPersonaBoard);
#endif

void BtlClosePersonaBoard(void)
{
    BtlBoardShut(g_btl_persona_board);
}
