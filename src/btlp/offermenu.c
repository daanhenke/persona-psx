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
#include <decomp/include_asm.h>
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

extern BtlEffectRow  g_btl_offer_rows[];
extern BtlEffect     g_btl_offer_menu;
extern u_char        g_btl_offer_name[];
extern BtlEffectRow  g_btl_offer_gauges[];
extern const u_char *g_btl_offer_labels[];
extern int           g_btl_talk_menu_effect;

extern int  BtlStockHolds(const BtlOffer *offer);
extern int  BtlEffectOpen(BtlEffect *e);
extern void BtlEffectSetKind(int slot, u_char kind);

#ifdef NON_MATCHING
void BtlOfferMenu(int slot)
{
    const u_char *name;
    BtlEffectRow *gauge;
    BtlEffectRow *link;
    BtlEffectRow *prev;
    BtlEffectRow *next;
    int           i;
    int           held;
    u_char        kind;
    u_char       *heldp;

    g_btl_offer_rows[1].text =
        &g_persona_data[g_btl_offer[slot].persona].level;
    name = g_persona_data[g_btl_offer[slot].persona].name;
    i = 0;
    while (*name != OFFER_END && i < OFFER_CHARS) {
        g_btl_offer_name[i] = *name;
        name++;
        i++;
    }
    g_btl_offer_name[i] = OFFER_END;
    /* Reached through a pointer so its address is worked out once and kept
       across the call. */
    heldp = &g_btl_offer_rows[4].kind;
    g_btl_offer_rows[3].text = g_btl_offer_name;
    *heldp = OFFER_LIT;
    held = BtlStockHolds(&g_btl_offer[slot]);
    if (held == 1) {
        kind = OFFER_HELD;
    } else {
        kind = OFFER_DIM;
    }
    i = 0;
    gauge = g_btl_offer_gauges;
    *heldp = kind;
    g_btl_offer_menu.next = (BtlEffectRow *)-1;
    g_btl_offer_gauges[0].next = (BtlEffectRow *)-1;
    g_btl_offer_menu.next = g_btl_offer_gauges;
    do {
        if (((g_btl_offer[slot].flags >> i) & 1) == 0) {
            kind = OFFER_DIM;
        } else {
            kind = OFFER_LIT;
        }
        gauge->kind = kind;
        gauge->x = (i >> 1) * OFFER_DX + OFFER_X0;
        gauge->row = OFFER_END;
        gauge->y = (i & 1) * OFFER_DY + OFFER_Y0;
        gauge->text = g_btl_offer_labels[i];
        i++;
        gauge++;
    } while (i < OFFER_GAUGES);

    i = 0;
    link = &g_btl_offer_gauges[1];
    prev = link - 1;
    do {
        i++;
        link->next = prev->next;
        prev->next = link;
        prev++;
        link++;
    } while (i < OFFER_GAUGES - 1);

    g_btl_offer_rows[0].next = g_btl_offer_gauges[i].next;
    g_btl_offer_gauges[i].next = &g_btl_offer_rows[0];
    next = g_btl_offer_rows[0].next;
    g_btl_offer_rows[0].next = &g_btl_offer_rows[1];
    g_btl_offer_rows[1].next = next;
    g_btl_offer_rows[3].next = next;
    g_btl_offer_rows[1].next = &g_btl_offer_rows[3];
    g_btl_offer_rows[4].next = next;
    g_btl_offer_rows[3].next = &g_btl_offer_rows[4];

    i = BtlEffectOpen(&g_btl_offer_menu);
    if (i != 0x100) {
        g_btl_talk_menu_effect = i;
        BtlEffectSetKind(i, 1);
    }
    BtlCursorShow(0);
}
#else
INCLUDE_ASM("btlp/nonmatchings/offermenu", BtlOfferMenu);
#endif

