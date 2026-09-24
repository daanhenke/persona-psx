/* Persona 1 (JP) - the offer on screen.  BTLP only.
 *   0x800747D8 BtlOfferMenu
 *
 * The offer is drawn as one effect. Its rows are the Persona's level and name,
 * a line that says whether the party already holds one, and eight gauges - two
 * columns of four - each lit or dimmed by its own bit of the offer's flags.
 *
 * The gauges are chained first, then the fixed rows are spliced in behind
 * them, and the whole thing is opened as g_btl_offer_menu. The cursor is taken
 * away as the menu goes up.
 *
 * The level row draws straight out of g_persona_data rather than a copy; only
 * the name is copied, because it has to be terminated.
 */
#include <decomp/types.h>
#include <persona/btlp/effect.h>
#include <persona/btlp/offer.h>
#include <persona/common/persona.h>
#include <persona/btlp/battle.h>

/* Gauges, and the fixed rows behind them. */
#define OFFER_GAUGES 8

/* The two columns are three cells apart and the four rows two, starting at
   (2, 3) in the eight-pixel steps a row is placed in. */
#define OFFER_X0 2
#define OFFER_DX 6
#define OFFER_Y0 3
#define OFFER_DY 2

/* Row kinds: lit and dimmed for a gauge, and the two the "already held" line
   takes. */
#define OFFER_LIT   1
#define OFFER_DIM   0x31
#define OFFER_HELD  0x11

/* A name is at most ten bytes and ends with this. */
#define OFFER_CHARS 10
#define OFFER_END   0xFF

/* The rows here are the short form of an effect row, as the stock menu's are:
   the link, the four bytes and the text, with no data after them. */
typedef struct BtlOfferRow {
    /* 0x0 */ struct BtlOfferRow *next;
    /* 0x4 */ u_char kind;
    /* 0x5 */ u_char row;
    /* 0x6 */ u_char x;
    /* 0x7 */ u_char y;
    /* 0x8 */ const u_char *text;
} BtlOfferRow;                  /* 0xC bytes */

/* The fixed rows: the heading, the Persona's level, its name, and the
   "already held" line. Each is its own object - the twelve bytes between the
   level and the name are something else, and nothing here reaches them. */
extern BtlOfferRow   g_btl_offer_row_title;
extern BtlOfferRow   g_btl_offer_row_level;
extern BtlOfferRow   g_btl_offer_row_name;
extern BtlOfferRow   g_btl_offer_row_held;
extern BtlEffect     g_btl_offer_menu;
extern u_char        g_btl_offer_name[];
extern BtlOfferRow   g_btl_offer_gauges[];
extern const u_char *g_btl_offer_labels[];
extern int           g_btl_talk_menu_effect;

extern int  BtlStockHolds(const BtlOffer *offer);

/* Hangs row x in after row p. */
#define LINK(p, x) ((x)->next = (p)->next, (p)->next = (x))

void BtlOfferMenu(int slot)
{
    const u_char *name;
    BtlOfferRow  *gauge;
    int           i;
    int           effect;
    u_char        kind;
    u_char       *heldp;

    g_btl_offer_row_level.text =
        &g_persona_data[g_btl_offer[slot].persona].level;
    name = g_persona_data[g_btl_offer[slot].persona].name;
    i = 0;
    while (*name != OFFER_END && i < OFFER_CHARS) {
        g_btl_offer_name[i] = *name;
        name++;
        i++;
    }
    g_btl_offer_name[i] = OFFER_END;
    g_btl_offer_row_name.text = g_btl_offer_name;
    /* Reached through a pointer so its address is worked out once and kept
       across the call. */
    heldp = &g_btl_offer_row_held.kind;
    *heldp = OFFER_LIT;
    if (BtlStockHolds(&g_btl_offer[slot]) == 1) {
        kind = OFFER_HELD;
    } else {
        kind = OFFER_DIM;
    }
    *heldp = kind;

    g_btl_offer_menu.next = (BtlEffectRow *)-1;
    g_btl_offer_gauges[0].next = (BtlOfferRow *)g_btl_offer_menu.next;
    g_btl_offer_menu.next = (BtlEffectRow *)&g_btl_offer_gauges[0];
    gauge = g_btl_offer_gauges;
    for (i = 0; i < OFFER_GAUGES; i++) {
        gauge->kind = ((g_btl_offer[slot].flags >> i) & 1) == 0 ? OFFER_DIM
                                                                  : OFFER_LIT;
        gauge->x = (i >> 1) * OFFER_DX + OFFER_X0;
        gauge->row = OFFER_END;
        gauge->y = (i & 1) * OFFER_DY + OFFER_Y0;
        gauge->text = g_btl_offer_labels[i];
        gauge++;
    }

    for (i = 0; i < OFFER_GAUGES - 1; i++) {
        LINK(&g_btl_offer_gauges[i], &g_btl_offer_gauges[i + 1]);
    }
    LINK(&g_btl_offer_gauges[i], &g_btl_offer_row_title);
    LINK(&g_btl_offer_row_title, &g_btl_offer_row_level);
    LINK(&g_btl_offer_row_level, &g_btl_offer_row_name);
    LINK(&g_btl_offer_row_name, &g_btl_offer_row_held);

    effect = BtlEffectOpen(&g_btl_offer_menu);
    if (effect != 0x100) {
        g_btl_talk_menu_effect = effect;
        BtlEffectSetKind(effect, 1);
    }
    BtlCursorShow(0);
}

