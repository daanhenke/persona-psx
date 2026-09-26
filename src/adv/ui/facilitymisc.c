/* Persona 1 (JP) - the facility screens' helpers.  ADV only.
 *
 * The stretch between the fusion screen and the shop counters: what the
 * velvet room, the shops and the other facilities lean on - lookups in the
 * Persona and fusion tables, counts, the money, and the screens' own steps.
 * Most of it is still the original's code.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <persona/common/persona.h>
#include <persona/common/char.h>
#include <persona/common/item.h>
#include <persona/common/menuctx.h>

#define g_persona_slots ((u_char *)0x801F2574)
#define g_item_list     ((u_short *)0x800EAE4C)
#define g_money         (*(u_int *)0x801F2674)
#define g_coins         (*(u_int *)0x801F2678)

#define SLOT_EMPTY    0xFF
#define FUSE_SPECIALS 40
#define COINS_MAX     99999999
#define ITEM_ID       0x1FF
#define ITEM_NO_SALE  0x1000

extern u_char D_800BA0E4[];     /* the fusion tables */
extern int  g_pad_pressed[];
extern void RunFrame(void);
extern short g_persona_data_step;
extern void ItemsCompact(void);
extern void ItemsClearPending(void);
extern void SlotSetPos(u_char slot, int attr, short x, short y);
extern void func_800A4E7C(void);
extern void func_800A52F0(void);
extern void func_800A5BCC(void);
extern void func_800A6308(void);
extern void func_800768F0(void);
extern void func_80077F8C(int a, int b);
extern void func_8007A62C(int a, int b);
extern int  MsgStep(void);

INCLUDE_ASM("adv/nonmatchings/ui/facilitymisc", func_800A1990);

INCLUDE_ASM("adv/nonmatchings/ui/facilitymisc", func_800A1C24);

/* Whether a Persona's definition lists `spell` among its first six. */
u_char PersonaDefHasSpell(short persona, short spell)
{
    int i;

    for (i = 0; i < 6; i++) {
        if (g_persona_defs[persona].raw[i] == spell) {
            return 1;
        }
    }
    return 0;
}

INCLUDE_ASM("adv/nonmatchings/ui/facilitymisc", func_800A1E7C);

INCLUDE_ASM("adv/nonmatchings/ui/facilitymisc", func_800A2140);

/* Whether a Persona is one of the forty the fusion tables treat specially. */
u_char FuseSpecialHas(short persona)
{
    int i;

    for (i = 0; i < FUSE_SPECIALS; i++) {
        if (D_800BA0E4[0x208 + i] == persona) {
            return 1;
        }
    }
    return 0;
}

INCLUDE_ASM("adv/nonmatchings/ui/facilitymisc", func_800A26DC);

INCLUDE_ASM("adv/nonmatchings/ui/facilitymisc", func_800A275C);

/* Fills g_item_list with the bag's items a shop will buy, and says how
   many there are. */
int ItemsListSellable(void)
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
    dst = g_item_list;
    for (; i < ITEM_SLOTS; i++) {
        v = *src++;
        if ((v & ITEM_ID) && !(g_item_defs[v & ITEM_ID].unk06 & ITEM_NO_SALE)) {
            *dst++ = v;
            n++;
        }
    }
    return n;
}

INCLUDE_ASM("adv/nonmatchings/ui/facilitymisc", func_800A2994);

INCLUDE_ASM("adv/nonmatchings/ui/facilitymisc", func_800A2A48);

INCLUDE_ASM("adv/nonmatchings/ui/facilitymisc", func_800A2CD0);

INCLUDE_ASM("adv/nonmatchings/ui/facilitymisc", func_800A2E9C);

INCLUDE_ASM("adv/nonmatchings/ui/facilitymisc", func_800A2FF8);

INCLUDE_ASM("adv/nonmatchings/ui/facilitymisc", func_800A3388);

INCLUDE_ASM("adv/nonmatchings/ui/facilitymisc", func_800A3984);

INCLUDE_ASM("adv/nonmatchings/ui/facilitymisc", func_800A3D0C);

INCLUDE_ASM("adv/nonmatchings/ui/facilitymisc", func_800A43BC);

/* How many of the sixteen Persona slots are filled. */
u_char PersonaSlotsCount(void)
{
    u_char *slots;
    u_char  i;
    u_char  n;

    slots = g_persona_slots;
    i = 0;
    n = 0;
    for (; i < 16; i++) {
        if (slots[i] != SLOT_EMPTY) {
            n++;
        }
    }
    return n;
}

/* Whether a Persona has reached its eighth spell slot. */
u_char PersonaSpellsFull(short n)
{
    return g_personas[n].slots >= 8;
}

/* Puts the facility cursor on its row: three rows 12 apart, the fourth
   further down. */
void FacilityCursorPlace(void)
{
    int n;

    n = g_menu->unk2D0.cur;
    switch (n) {
    case 0:
    case 1:
    case 2:
        SlotSetPos(1, 0x42, 0x48, n * 12 + 0x3C);
        break;
    case 3:
        SlotSetPos(1, 0x42, 0x48, 0x6C);
        break;
    }
}

INCLUDE_ASM("adv/nonmatchings/ui/facilitymisc", func_800A47F8);

INCLUDE_ASM("adv/nonmatchings/ui/facilitymisc", func_800A4BD0);

INCLUDE_ASM("adv/nonmatchings/ui/facilitymisc", func_800A4C98);

#ifdef NON_MATCHING
/* Whether a character may take a Persona: it must answer them at all, and
   they must be at its level. */
/* 96.14%: the image keeps v0 free and puts the Persona offset in a1;
   this build uses v0 and a0 for the same values. */
u_char CharCanUsePersona(short chr, short persona)
{
    Char *c;

    c = &g_chars[chr];
    if ((g_persona_defs[persona].bond >> ((c->key - 1) * 2)) & 3) {
        return g_persona_defs[persona].level <= c->unk56;
    }
    return 0;
}
#else
INCLUDE_ASM("adv/nonmatchings/ui/facilitymisc", CharCanUsePersona);
#endif

void ShopStep2(void)
{
    switch (g_persona_data_step) {
    case 0:
        func_800A4E7C();
        break;
    case 1:
        func_800A52F0();
        break;
    }
}

INCLUDE_ASM("adv/nonmatchings/ui/facilitymisc", func_800A4E7C);

INCLUDE_ASM("adv/nonmatchings/ui/facilitymisc", func_800A52F0);

INCLUDE_ASM("adv/nonmatchings/ui/facilitymisc", func_800A5560);

/* Keeps the message running until any button is pressed. */
void MsgWaitPress(void)
{
    goto check;
loop:
    MsgStep();
check:
    RunFrame();
    if (g_pad_pressed[0] == 0) goto loop;
    RunFrame();
}

/* How many coins the money buys, at a hundred a coin, up to what the coin
   count can still hold. */
u_int CoinsAffordable(void)
{
    u_int room;
    u_int n;

    n = g_money / 100;
    room = COINS_MAX - g_coins;
    if (room < n) {
        return room;
    }
    return n;
}

void CoinExchangeStep(void)
{
    switch (g_persona_data_step) {
    case 0:
        func_800A5BCC();
        break;
    case 1:
        func_800A6308();
        break;
    }
}

INCLUDE_ASM("adv/nonmatchings/ui/facilitymisc", func_800A5BCC);

INCLUDE_ASM("adv/nonmatchings/ui/facilitymisc", func_800A6308);

INCLUDE_ASM("adv/nonmatchings/ui/facilitymisc", func_800A6674);

INCLUDE_ASM("adv/nonmatchings/ui/facilitymisc", func_800A6728);

INCLUDE_ASM("adv/nonmatchings/ui/facilitymisc", func_800A6788);

INCLUDE_ASM("adv/nonmatchings/ui/facilitymisc", func_800A6E50);

INCLUDE_ASM("adv/nonmatchings/ui/facilitymisc", func_800A6F2C);

INCLUDE_ASM("adv/nonmatchings/ui/facilitymisc", func_800A6FFC);

INCLUDE_ASM("adv/nonmatchings/ui/facilitymisc", func_800A7118);

INCLUDE_ASM("adv/nonmatchings/ui/facilitymisc", func_800A7578);

INCLUDE_ASM("adv/nonmatchings/ui/facilitymisc", func_800A76F4);

INCLUDE_ASM("adv/nonmatchings/ui/facilitymisc", func_800A79CC);

INCLUDE_ASM("adv/nonmatchings/ui/facilitymisc", func_800A7BD0);

INCLUDE_ASM("adv/nonmatchings/ui/facilitymisc", func_800A7D7C);

INCLUDE_ASM("adv/nonmatchings/ui/facilitymisc", func_800A8104);

void ShopScreenOpen(void)
{
    func_800768F0();
    func_80077F8C(3, 1);
    func_8007A62C(3, 8);
}

INCLUDE_ASM("adv/nonmatchings/ui/facilitymisc", func_800A8200);

INCLUDE_ASM("adv/nonmatchings/ui/facilitymisc", func_800A8448);

INCLUDE_ASM("adv/nonmatchings/ui/facilitymisc", func_800A8928);

INCLUDE_ASM("adv/nonmatchings/ui/facilitymisc", func_800A8B88);

INCLUDE_ASM("adv/nonmatchings/ui/facilitymisc", func_800A91C8);

INCLUDE_ASM("adv/nonmatchings/ui/facilitymisc", func_800A95B8);

INCLUDE_ASM("adv/nonmatchings/ui/facilitymisc", func_800A97A4);

INCLUDE_ASM("adv/nonmatchings/ui/facilitymisc", func_800A9868);
