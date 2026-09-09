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
#include <decomp/include_asm.h>
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

extern BtlEffectRow g_btl_stock_rows[];
extern u_char       g_btl_stock_names[][STOCK_NAME];
/* Only the chain head is touched here; the rest of the record is the
   effect's own state. */
extern BtlEffectRow *g_btl_stock_effect;
extern int          g_btl_talk_effect;

extern int  BtlEffectOpen(BtlEffect *e);
extern void BtlEffectSetKind(int slot, u_char kind);

#ifdef NON_MATCHING
void BtlTalkStockMenu(void)
{
    BtlEffectRow *row;
    u_char       *name;
    u_char       *p;
    const u_char *stock;
    int           i;
    int           j;

    row = g_btl_stock_rows;
    i = 0;
    name = g_btl_stock_names[0];
    stock = g_persona_stock;
    g_btl_stock_effect = (BtlEffectRow *)-1;
    do {
        row->kind = STOCK_ROW_KIND;
        row->row = i;
        row->x = (i % STOCK_COLS) * STOCK_DX + STOCK_X0;
        row->y = (i / STOCK_COLS) * STOCK_DY + STOCK_Y0;

        j = 0;
        p = name;
        while (g_persona_data[*stock].name[j] != STOCK_END && j < STOCK_CHARS) {
            *p = g_persona_data[*stock].name[j];
            j++;
            p++;
        }
        name[j] = STOCK_END;
        row->text = name;

        name += STOCK_NAME;
        stock++;
        i++;
        row->next = g_btl_stock_effect;
        g_btl_stock_effect = row;
        row++;
    } while (i < STOCK_SLOTS);

    g_btl_talk_effect = BtlEffectOpen((BtlEffect *)&g_btl_stock_effect);
    BtlEffectSetKind(g_btl_talk_effect, STOCK_ROW_KIND);
    BtlEffectSelect(g_btl_talk_effect);
}
#else
INCLUDE_ASM("btlp/nonmatchings/stockmenu", BtlTalkStockMenu);
#endif

