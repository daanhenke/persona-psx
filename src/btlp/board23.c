/* Persona 1 (JP) - the board of the kinds of enemy in the fight.  BTLP only.
 *   0x800A95BC BtlOpenBoard23  0x800A98A8 BtlCloseBoard23
 *
 * R1 on the command picker puts it up and holds it until a key comes. The
 * open counts the kinds on the field, three at most in the order they are
 * met: every fighter record with a key adds one to the first entry that is
 * still free or already holds that key. Each entry then fills a row of the
 * board - the arcana's label, the Persona's name and how many there are - and
 * a row nobody took is blanked. Then the board goes up; the close takes it
 * down.
 *
 * The board's cells are two-dimensional tables, a row of cells per entry, and
 * that is load-bearing: a blank written as `counts[i][0]` reaches the table
 * through its symbol, where `counts[i * 2]` loads the symbol into a register
 * of its own, which pairs with the call's and lifts both out of the loop.
 */
#include <decomp/types.h>
#include <decomp/libc.h>
#include <persona/common/persona.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/board.h>
#include <persona/btlp/number.h>
#include <persona/btlp/object.h>

/* The rows the board has, and the fighter records searched for them. */
#define BOARD23_ROWS    3
#define BOARD23_RECORDS 9

/* A row nobody took, and a blanked cell run. */
#define BOARD23_FREE    0xFF

/* The cells of a row's label, name and count, and the count's width. */
#define BOARD23_LABEL_CELLS 6
#define BOARD23_NAME_CELLS  10
#define BOARD23_COUNT_CELLS 2
#define BOARD23_COUNT_WIDTH 1

/* One kind and how many of it are on the field. */
typedef struct {
    u_char key;
    u_char count;
} BtlKindCount;

extern BtlObj            *g_btl_board23;
extern BtlBoardDef        g_btl_board23_defs[];
extern const long         g_btl_board23_pos[];

extern BtlKindCount       g_btl_board23_kinds[];
extern u_char             g_btl_board23_arcana[][BOARD23_LABEL_CELLS];
extern u_char             g_btl_board23_names[][BOARD23_NAME_CELLS];
extern u_char             g_btl_board23_counts[][BOARD23_COUNT_CELLS];

void BtlOpenBoard23(void)
{
    int i;
    int j;

    g_btl_board23_kinds[0].key = BOARD23_FREE;
    g_btl_board23_kinds[1].key = BOARD23_FREE;
    g_btl_board23_kinds[2].key = BOARD23_FREE;
    g_btl_board23_kinds[0].count = 0;
    g_btl_board23_kinds[1].count = 0;
    g_btl_board23_kinds[2].count = 0;
    for (i = 0; i < BOARD23_RECORDS; i++) {
        for (j = 0; j < BOARD23_ROWS; j++) {
            if (g_btl_combatants[i].c.key != 0
                && (g_btl_board23_kinds[j].key == BOARD23_FREE
                    || g_btl_board23_kinds[j].key
                           == g_btl_combatants[i].c.key)) {
                g_btl_board23_kinds[j].key = g_btl_combatants[i].c.key;
                g_btl_board23_kinds[j].count++;
                break;
            }
        }
    }
    for (i = 0; i < BOARD23_ROWS; i++) {
        if (g_btl_board23_kinds[i].key != BOARD23_FREE) {
            memcpy(g_btl_board23_arcana[i], g_btl_arcana_labels[
                g_persona_data[g_btl_board23_kinds[i].key].arcana],
                BOARD23_LABEL_CELLS);
            memcpy(g_btl_board23_names[i],
                g_persona_data[g_btl_board23_kinds[i].key].name,
                BOARD23_NAME_CELLS);
            BtlDrawNumberAlt(g_btl_board23_counts[i],
                             g_btl_board23_kinds[i].count,
                             BOARD23_COUNT_WIDTH);
        } else {
            g_btl_board23_arcana[i][0] = BOARD23_FREE;
            g_btl_board23_names[i][0] = BOARD23_FREE;
            g_btl_board23_counts[i][0] = BOARD23_FREE;
        }
    }
    g_btl_board23 = BtlBoardOpen(g_btl_board23_defs, g_btl_board23_pos);
}

void BtlCloseBoard23(void)
{
    BtlBoardShut(g_btl_board23);
}
