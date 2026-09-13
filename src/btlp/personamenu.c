/* Persona 1 (JP) - the two pages of the Persona board.  BTLP only.
 *   0x800A3B38 BtlPersonaSpellUpdate  0x800A3D68 BtlPersonaSwapUpdate
 *
 * The spell order's board lists the acting member's Persona's spells in two
 * columns, and BtlPersonaSpellUpdate is one frame of its cursor. The
 * directions walk g_btl_persona_spell_nav - up, down and across - passing over
 * rows with no spell in them; the cursor is stood at the row's spot, and unless
 * help is switched off the spell's help line goes up along the bottom. It
 * answers the row on a confirm, -1 on a cancel, -2 on the abort key and
 * BTL_PICK_WAIT otherwise.
 *
 * BtlPersonaSwapUpdate is the page that changes Persona. It gathers the ones
 * the member carries besides the one equipped into g_btl_swap_personas, with
 * the list slot each came from in g_btl_swap_slots, and up and down move
 * between the two. Every frame it writes the chosen one's spell names into the
 * board's lines, packed up so no row is left empty between two spells, and it
 * answers the list slot on a confirm, with the other answers as above.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <decomp/libc.h>
#include <persona/common/spell.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/input.h>
#include <persona/btlp/menu.h>
#include <persona/btlp/object.h>
#include <persona/btlp/pick.h>
#include <persona/btlp/round.h>
#include <persona/btlp/sound.h>
#include <persona/btlp/stats.h>
#include <persona/btlp/text.h>

#define PAD_UP   0x1000
#define PAD_DOWN 0x4000
#define PAD_DIRS 0xF000

/* The three ways the spell cursor's table can be walked. */
#define NAV_UP   0
#define NAV_DOWN 1
#define NAV_SIDE 2
#define NAV_WAYS 3

/* The board's lines: a spell's name and the byte that ends it. */
#define SPELL_LINE 11
#define LINE_EMPTY 0xFF

/* The Personas a member carries, and the two the swap can offer. */
#define CARRIED    3
#define SWAP_ROWS  2
#define SWAP_NONE  0xFF

/* The cursor's cells and how far left of a spot it stands. */
#define CURSOR_CELLS 14
#define CURSOR_NUDGE 7

/* Where the help line goes, and the answer to the abort key. */
#define HELP_X     0x10
#define HELP_Y     0x8C
#define PICK_ABORT (-2)

u_char g_btl_persona_spell_nav[BTL_STATS_SPELLS][NAV_WAYS] = {
    { 6, 2, 1 }, { 5, 3, 0 }, { 0, 4, 3 }, { 1, 5, 2 },
    { 2, 6, 5 }, { 3, 1, 4 }, { 4, 0, 6 },
};

BtlMenuSpot g_btl_persona_spell_spots[BTL_STATS_SPELLS] = {
    { -0x80, -0x10 }, { 0x10, -0x10 }, { -0x80, -0x04 }, { 0x10, -0x04 },
    { -0x80, 0x08 },  { 0x10, 0x08 },  { -0x80, 0x14 },
};

BtlMenuSpot g_btl_persona_swap_spots[SWAP_ROWS] = {
    { -0x88, 0x08 }, { -0x88, 0x14 },
};

/* The two Personas the swap offers and the list slots they came from. */
extern u_char g_btl_swap_personas[SWAP_ROWS];
extern u_char g_btl_swap_slots[SWAP_ROWS];

/* One help line per spell id. */
extern u_char *g_btl_spell_help[];

int BtlPersonaSpellUpdate(short *row)
{
    BtlGfxCell *cell;
    int         dir;
    int         keys;
    int         i;

    dir = -1;
    keys = BtlMenuKey();
    if (keys & PAD_DIRS) {
        BtlSePlay(1, 0);
    }
    if (keys & PAD_UP) {
        dir = NAV_UP;
    }
    if (keys & PAD_DOWN) {
        dir = NAV_DOWN;
    }
    if (keys & PAD_LEFT) {
        dir = NAV_SIDE;
    }
    if (keys & PAD_RIGHT) {
        dir = NAV_SIDE;
    }
    while (dir >= 0) {
        *row = g_btl_persona_spell_nav[*row][dir];
        if (g_btl_persona_spell_lines[*row][0] != LINE_EMPTY) {
            break;
        }
    }
    for (cell = g_btl_menu_cursor, i = 0; i < CURSOR_CELLS; i++, cell++) {
        cell->x = g_btl_persona_spell_spots[*row].x - CURSOR_NUDGE;
        cell->y = g_btl_persona_spell_spots[*row].y;
    }
    if (g_btl_no_help == 0) {
        BtlOpenMessage(0, 0,
                       g_btl_spell_help[g_btl_personas[BtlActorPersona(g_btl_actor_turn)]
                                            .spell[*row]],
                       HELP_X, HELP_Y);
    } else {
        BtlCloseMessage(0);
    }
    if (g_btl_pad1_edge & g_btl_key_confirm) {
        return *row;
    }
    if (g_btl_pad1_edge & g_btl_key_cancel) {
        return -1;
    }
    if (g_btl_pad1_edge & g_btl_key_abort) {
        return PICK_ABORT;
    }
    return BTL_PICK_WAIT;
}

/* 95.89%, registers only: the name fill's packed line pointer takes t1 and
   the row index t0, the other way round from the image, and the pointer is
   loaded before the Persona id rather than after it. Reordering the setup,
   taking the id into a local and walking the marks by pointer all leave it
   there or lose ground. */
#ifdef NON_MATCHING
int BtlPersonaSwapUpdate(short *row)
{
    BtlStats   *p;
    BtlGfxCell *cell;
    u_char     *line;
    int         equipped;
    int         pick;
    int         keys;
    int         id;
    int         n;
    int         i;

    equipped = BtlActorPersona(g_btl_actor_turn);
    g_btl_swap_personas[0] = SWAP_NONE;
    g_btl_swap_personas[1] = SWAP_NONE;
    for (i = 0, n = 0; i < CARRIED; i++) {
        id = g_btl_actors[g_btl_actor_turn].c.list[i];
        if (id != SWAP_NONE && id != equipped) {
            g_btl_swap_personas[n] = id;
            g_btl_swap_slots[n] = i;
            n++;
        }
    }

    pick = *row;
    keys = (u_short)BtlMenuKey();
    if (keys & (PAD_UP | PAD_DOWN)) {
        BtlSePlay(1, 0);
    }
    if (keys & PAD_UP) {
        pick ^= 1;
    }
    if (keys & PAD_DOWN) {
        pick ^= 1;
    }
    if (g_btl_swap_personas[pick] != SWAP_NONE) {
        *row = pick;
    }

    line = (u_char *)g_btl_persona_spell_lines;
    p = &g_btl_personas[g_btl_swap_personas[*row]];
    for (i = 0; i < BTL_STATS_SPELLS; i++) {
        g_btl_persona_spell_lines[i][0] = LINE_EMPTY;
        if (p->spell[i] != 0) {
            memcpy(line, g_spell_data[p->spell[i]].name, SPELL_NAME_CELLS);
            line += SPELL_LINE;
        }
    }

    for (cell = g_btl_menu_cursor, i = 0; i < CURSOR_CELLS; i++, cell++) {
        cell->x = g_btl_persona_swap_spots[*row].x - CURSOR_NUDGE;
        cell->y = g_btl_persona_swap_spots[*row].y;
    }
    if (g_btl_pad1_edge & g_btl_key_confirm) {
        return g_btl_swap_slots[*row];
    }
    if (g_btl_pad1_edge & g_btl_key_cancel) {
        return -1;
    }
    if (g_btl_pad1_edge & g_btl_key_abort) {
        return PICK_ABORT;
    }
    return BTL_PICK_WAIT;
}
#else
INCLUDE_ASM("btlp/nonmatchings/personamenu", BtlPersonaSwapUpdate);
#endif
