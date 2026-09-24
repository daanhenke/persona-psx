/* Persona 1 (JP) - the Persona stock list, during a negotiation.  BTLP only.
 *   0x80067BE8 BtlTalkStockMenu
 *
 * The twelve stock slots are put on screen as one effect: a row record per
 * slot, three across and four down, each carrying the Persona's name copied
 * out of g_persona_data. The rows are chained newest-first, so the list ends
 * up with slot zero at the head, and opening the effect starts it on row
 * zero - the first slot.
 *
 * A name is at most ten bytes and ends with 0xFF, which is also what an empty
 * one gets.
 */
#include <decomp/types.h>
#include <persona/btlp/effect.h>
#include <persona/common/persona.h>

/* Stock slots, and the grid they are laid out in. */
#define STOCK_SLOTS 12
#define STOCK_COLS  3

/* Where the grid starts and how far apart the cells are, in the eight-pixel
   steps the row records are measured in. */
#define STOCK_X0   4
#define STOCK_DX   11
#define STOCK_Y0   1
#define STOCK_DY   2

/* Bytes a name gets, and what ends one. */
#define STOCK_NAME  0xC
#define STOCK_CHARS 10
#define STOCK_END   0xFF

/* The drawer the rows are given. */
#define STOCK_ROW_KIND 1

/* The stock rows are the short form of an effect row: the link, the four
   bytes and the text, with no data after them. */
typedef struct BtlStockRow {
    /* 0x0 */ struct BtlStockRow *next;
    /* 0x4 */ u_char kind;
    /* 0x5 */ u_char row;
    /* 0x6 */ u_char x;
    /* 0x7 */ u_char y;
    /* 0x8 */ const u_char *text;
} BtlStockRow;                  /* 0xC bytes */

extern BtlStockRow g_btl_stock_rows[];
extern u_char       g_btl_stock_names[][STOCK_NAME];
/* Only the chain head is touched here; the rest of the record is the
   effect's own state. */
extern BtlStockRow *g_btl_stock_effect;
extern int          g_btl_talk_effect;

/* Indexed by i throughout: the name and stock walkers are loop.c's, which is
   the order the image sets them up in, behind its own lifts. The chain head's
   address is one value in and after the loop - it goes straight on to
   BtlEffectOpen - and the row steps before the counter, which puts the text
   pointer's step first. */
void BtlTalkStockMenu(void)
{
    BtlStockRow  *row;
    int           i;
    int           j;
    BtlStockRow **head;

    row = g_btl_stock_rows;
    g_btl_stock_effect = (BtlStockRow *)-1;
    i = 0;
    do {
        row->kind = STOCK_ROW_KIND;
        row->row = i;
        row->x = (i % STOCK_COLS) * STOCK_DX + STOCK_X0;
        row->y = (i / STOCK_COLS) * STOCK_DY + STOCK_Y0;

        j = 0;
        while (g_persona_data[g_persona_stock[i]].name[j] != STOCK_END
               && j < STOCK_CHARS) {
            g_btl_stock_names[i][j] = g_persona_data[g_persona_stock[i]].name[j];
            j++;
        }
        g_btl_stock_names[i][j] = STOCK_END;
        row->text = g_btl_stock_names[i];

        head = &g_btl_stock_effect;
        row->next = *head;
        *head = row;
        row++;
        i++;
    } while (i < STOCK_SLOTS);

    g_btl_talk_effect = BtlEffectOpen((BtlEffect *)head);
    BtlEffectSetKind(g_btl_talk_effect, STOCK_ROW_KIND);
    BtlEffectSelect(g_btl_talk_effect);
}

