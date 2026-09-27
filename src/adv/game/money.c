/* Persona 1 (JP) - the party's money.
 *
 *   ADV @ 0x800AAFD4
 *
 * The counter lives in the save-game work area, eight bytes before g_items,
 * and this is the only thing that adds to it. The cap is 999,999,999 and the
 * comparison against it is unsigned, so a total that would overflow past the
 * cap sticks there rather than wrapping.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <persona/common/item.h>
#include <persona/common/persona.h>
#include <persona/common/spell.h>
#include <persona/common/tilemap.h>
#include <persona/common/slot.h>

#define g_money (*(u_int *)0x801F2674)

#define MONEY_MAX 999999999

void MoneyAdd(u_int amount)
{
    g_money = g_money + amount;
    if (g_money > MONEY_MAX) {
        g_money = MONEY_MAX;
    }
}

/* The rest of the unit: the two spending counters, the sell list, the price
   row, the slot clear the facility screens share, and the rules for which
   spell a fused Persona may inherit. */

/* The casino's coins, the word after the money. */
#define g_coins (*(u_int *)0x801F2678)

/* ItemDef.unk06: the shop's sell page lists the item. */
#define ITEM_SHOP_SELL 0x2000

void MoneySpend(u_int amount)
{
    g_money = g_money - amount;
}

void CoinsSpend(u_int amount)
{
    g_coins = g_coins - amount;
}

/* Stages every held item the shop's sell page lists into g_items_pending;
   returns how many. */
int ItemsListShopSell(void)
{
    u_short *src;
    u_short *dst;
    u_short  v;
    int      i;
    int      n;

    ItemsCompact();
    ItemsClearPending();
    i = 0;
    n = 0;
    src = g_items;
    dst = g_items_pending;
    for (; i < ITEM_SLOTS; i++) {
        v = *src++;
        if ((v & ITEM_ID) && (g_item_defs[v & ITEM_ID].unk06 & ITEM_SHOP_SELL)) {
            *dst++ = v;
            n++;
        }
    }
    return n;
}

/* The price of staged entry `n` and the party's money, in the lower
   window. */
void ItemDrawPrice(short n)
{
    u_short *p;

    TileMapFillRect(&g_tilemap2[9 * MAP_W + 17], 0, 9, 1, MAP_W);
    TileMapFillRect(&g_tilemap2[11 * MAP_W + 17], 0, 9, 1, MAP_W);
    p = &g_items_pending[n];
    if ((*p & ITEM_ID) && (*p >> 9)) {
        TileMapWriteRowRev(g_hud_digits, &g_tilemap2[11 * MAP_W + 24],
                           GLYPH_DIGIT0,
                           FormatDecimal(g_item_defs[*p & ITEM_ID].price,
                                         g_hud_digits, 9));
    }
    TileMapWriteRowRev(g_hud_digits, &g_tilemap2[13 * MAP_W + 24],
                       GLYPH_DIGIT0, FormatDecimal(g_money, g_hud_digits, 9));
    g_tilemap2[11 * MAP_W + 15] = 0xD0;
}

void FacilitySlotsClear(void)
{
    SlotClear(3);
    SlotClear(8);
    SlotClear(9);
    SlotClear(0xA);
    SlotClear(0xB);
    SlotClear(0xC);
    SlotClear(0xD);
}

/* The inheritance tables: per arcana group a row of ten spell classes
   (0xFF ends a row), per class a (first, count) run into the spell list. */
extern u_char g_arcana_fuse_group[];
extern u_char g_fuse_group_classes[];
extern u_char g_fuse_spell_runs[];
extern u_char g_fuse_spell_list[];

short PersonaDataSpellIndex(short id, short spell);
short FuseSpellAllowed(short persona, short spell);

/* The first spell of Persona data `src` that `persona` may inherit, by the
   group of src's arcana: the spell into *spell and how far down the group's
   list it came into *rank. */
/* 99.44%: two things left. i and n trade s4/s2 (global-alloc priority; the
   do-while round n's reset is what put end in s3), and the image loads the
   run's first entry straight into j and adds a copy of it to the count,
   where this loads into first and cse folds it into j. */
#ifdef NON_MATCHING
short FuseInheritSpell(short persona, short src, short *spell, short *rank)
{
    short i;
    short j;
    short end;
    short n;
    short group;
    short first;

    do {
        n = 0;
    } while (0);
    group = g_arcana_fuse_group[g_persona_data[src].arcana - 1] - 1;
    *rank = 0xFF;
    *spell = 0xFF;
    for (i = 0; i < 10; i++) {
        if (g_fuse_group_classes[i] == 0xFF) {
            break;
        }
        first = g_fuse_spell_runs[g_fuse_group_classes[group * 10 + i] * 2];
        end = first + g_fuse_spell_runs[g_fuse_group_classes[group * 10 + i] * 2 + 1];
        for (j = first; j < end; j++, n++) {
            if (PersonaDataSpellIndex(src, g_fuse_spell_list[j]) != -1
                && FuseSpellAllowed(persona, g_fuse_spell_list[j])) {
                *spell = g_fuse_spell_list[j];
                *rank = n;
                return 1;
            }
        }
    }
    return 0;
}
#else
INCLUDE_ASM("adv/nonmatchings/game/money", FuseInheritSpell);
#endif

/* Where `spell` sits in Persona data `id`'s spell list, or -1. */
short PersonaDataSpellIndex(short id, short spell)
{
    PersonaData *p;
    int          i;

    p = &g_persona_data[id];
    for (i = 0; i < 5; i++) {
        if (p->spell[i] == spell) {
            return i;
        }
    }
    return -1;
}

/* The same in Persona `id`'s own list; FuseSpellAllowed has it inline. */
inline short PersonaDefSpellIndex(short id, short spell)
{
    int i;

    for (i = 0; i < 6; i++) {
        if (g_persona_defs[id].raw[i] == spell) {
            return i;
        }
    }
    return -1;
}

/* Whether `persona` may take `spell` from a fusion: never one it already
   knows, and each class of spell only on the types and kinds it suits. */
short FuseSpellAllowed(short persona, short spell)
{
    int ok;

    ok = 0;
    if (PersonaDefSpellIndex(persona, spell) != -1) {
        return ok;
    }
    switch (g_spell_data[spell].kind & 0x1E) {
    case 0:
        if (g_persona_defs[persona].pad27[0] == 0xC) {
            break;
        }
        ok = 1;
        break;
    case 2:
        if (g_persona_defs[persona].pad27[0] == 0xB) {
            break;
        }
        ok = 1;
        break;
    case 4:
        if (g_persona_defs[persona].pad27[0] == 0xE) {
            break;
        }
        ok = 1;
        break;
    case 6:
        if (g_persona_defs[persona].pad27[0] == 0xD) {
            break;
        }
        ok = 1;
        break;
    case 16:
    kind16:
        switch (g_persona_defs[persona].kind) {
        case 2:
        case 0x14:
            break;
        default:
            ok = 1;
            break;
        }
        break;
    case 24:
    kind24:
        if (g_persona_defs[persona].kind == 0xC
            || g_persona_defs[persona].kind == 0xD
            || g_persona_defs[persona].kind == 0xF
            || g_persona_defs[persona].kind == 0x10
            || g_persona_defs[persona].kind == 0x12) {
            break;
        }
        ok = 1;
        break;
    case 22:
        if (g_persona_defs[persona].kind == 5) {
            break;
        }
        goto kind16;
    case 28:
    case 30:
        if (g_persona_defs[persona].kind == 7
            || g_persona_defs[persona].kind == 0xB) {
            break;
        }
        goto kind24;
    case 8:
    case 10:
    case 12:
    case 14:
    case 18:
    case 20:
    case 26:
        ok = 1;
        break;
    }
    return ok;
}
