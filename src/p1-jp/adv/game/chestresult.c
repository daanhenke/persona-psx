/* Persona 1 (JP) - what a chest holds, and what it tells the player.
 *
 *   ADV @ 0x8008157C, 0x80081864, 0x80081A20, 0x80081BB0
 *
 * The search handler at 0x8007E814 walks the player onto the tile, finds the
 * actor standing there and reads its flag. A flag already set means the chest
 * has been taken, and AdvChestShowEmpty says so; otherwise the flag is set,
 * the opening animation plays, and what came out decides which of the other
 * two runs.
 *
 * AdvChestApply is what decides that. The object's record says what it holds,
 * and most kinds are traps: a fraction of the party leader's HP, or a poison.
 * Kinds 9 and 10 are a trap and a reward at once and reach the reward by
 * falling into the plain item and money cases. What it returns is what the
 * caller shows.
 *
 * The three display routines work the same way: fill the sprite's cells with
 * the text, put it in slot 62 over the actor's head, hold it for 48 frames and
 * then squeeze it away over 16 more. The sprite is nudged left by half its
 * width so it stays centred, which for the money is four pixels per digit
 * rather than a fixed offset.
 */
#include <types.h>
#include <libgs.h>
#include <persona/adv/actor.h>
#include <persona/common/char.h>
#include <persona/common/item.h>
#include <persona/common/slot.h>
#include <persona/common/status.h>

extern u_char g_party[];

/* Reached by hardcoded address rather than through the linker symbol. */
#define g_slots ((Slot *)0x800DC10C)

#define EFFECT_SLOT 62
#define HOLD_FRAMES 0x30
#define SPIN_FRAMES 0x10
#define SPIN_STEP   0x16000
#define SHRINK_STEP 0xF0

extern Slot  *g_slot_cur;
extern short  g_cam_x;
extern short  g_cam_y;

extern u_short g_adv_chest_item;    /* id, indexing g_item_defs */
extern u_short g_adv_chest_count;
extern u_int   g_adv_chest_money;

extern GsCELL      g_adv_chest_item_cells[];
extern GsCELL      g_adv_chest_count_cells[];
extern GsCELL      g_adv_chest_money_cells[];
extern GsCELL      g_adv_chest_empty_cells[];
extern void        g_adv_chest_item_def;
extern void        g_adv_chest_money_def;
extern void        g_adv_chest_empty_def;
/* The party HUD has its own copy of the same five bytes, called str_empty. */
extern const u_char str_chest_empty[];

extern u_char g_hud_digits[];

extern short FormatDecimal(u_int value, u_char *dst, u_short width);
extern void  CellsWriteRow(GsCELL *dst, const u_char *src, u_char page,
                           u_short count);
extern void  CellsWriteDigitsRev(GsCELL *dst, const u_char *src,
                                 u_short count);
extern void  CellsClear(GsCELL *dst, u_char count);
extern void  AdvRunFrame(void);

/* One record per searchable object in the scene pack, indexed by the actor's
   number less eight. The search handler reads `flag` through a base of its
   own; only the kind and the payload are wanted here. */
typedef struct {
    /* 0x0 */ short    unk0;
    /* 0x2 */ u_char   kind;
    /* 0x3 */ u_char   pad03;
    /* 0x4 */ short    flag;
    /* 0x6 */ u_short  payload;
    /* 0x8 */ u_char   pad08[2];
} AdvChest;                         /* 10 bytes */

#define g_chest_defs ((AdvChest *)0x80100BD0)

/* What the kind byte selects. Most of them are traps on the party leader; 9
   and 10 are a trap and a reward together, and reach the reward by falling
   into the plain cases. 8 and 11 are inside the jump table's range but do
   nothing. */
#define CHEST_ITEM          0
#define CHEST_MONEY         1
#define CHEST_TRAP_QUARTER  2   /* takes a quarter of the leader's HP */
#define CHEST_TRAP_HALF     3
#define CHEST_TRAP_PARTY    4   /* every member down to 1 HP          */
#define CHEST_TRAP_POISON   5
#define CHEST_TRAP_ONE      6   /* a single point                     */
#define CHEST_TRAP_TO_ONE   7
#define CHEST_TRAP_ITEM     9   /* a tenth of the HP, then the item   */
#define CHEST_TRAP_MONEY    10

/* An item payload packs the id in the low nine bits and the count above. */
#define CHEST_ITEM_ID  0x1FF
#define CHEST_ITEM_N   9

/* Money is stored in hundreds. */
#define CHEST_MONEY_UNIT 100

/* What the caller switches on: nothing, an item, or money. */
#define CHEST_SHOWS_NOTHING 0
#define CHEST_SHOWS_ITEM    1
#define CHEST_SHOWS_MONEY   2

extern void ItemsAdd(u_short id, u_short count);
extern void MoneyAdd(u_int amount);

u_char AdvChestApply(u_char chest)
{
    AdvChest *d;
    Char *c;
    int hp;
    u_char shows;
    u_char i;

    d = &g_chest_defs[chest];
    shows = CHEST_SHOWS_NOTHING;
    switch (d->kind) {
    case CHEST_TRAP_QUARTER:
        c = &g_chars[g_party[0]];
        hp = c->hp;
        hp -= hp / 4;
        c->hp = hp;
        break;
    case CHEST_TRAP_HALF:
        c = &g_chars[g_party[0]];
        hp = c->hp;
        hp -= hp / 2;
        c->hp = hp;
        break;
    case CHEST_TRAP_PARTY:
        for (i = 0; i < CHAR_COUNT; i++) {
            if (g_party[i] != 0xFF) {
                g_chars[g_party[i]].hp = 1;
            }
        }
        return CHEST_SHOWS_NOTHING;
    case CHEST_TRAP_POISON:
        g_chars[g_party[0]].status = STATUS_POISON;
        return CHEST_SHOWS_NOTHING;
    case CHEST_TRAP_ONE:
        c = &g_chars[g_party[0]];
        hp = c->hp;
        if (hp != 1) {
            c->hp = hp - 1;
        }
        break;
    case CHEST_TRAP_TO_ONE:
        g_chars[g_party[0]].hp = 1;
        return CHEST_SHOWS_NOTHING;
    case CHEST_TRAP_ITEM:
        c = &g_chars[g_party[0]];
        hp = c->hp;
        hp -= hp / 10;
        c->hp = hp;
        /* falls through */
    case CHEST_ITEM:
        g_adv_chest_item = d->payload & CHEST_ITEM_ID;
        g_adv_chest_count = d->payload >> CHEST_ITEM_N;
        ItemsAdd(g_adv_chest_item, g_adv_chest_count);
        shows = CHEST_SHOWS_ITEM;
        break;
    case CHEST_TRAP_MONEY:
        c = &g_chars[g_party[0]];
        hp = c->hp;
        hp -= hp / 10;
        c->hp = hp;
        /* falls through */
    case CHEST_MONEY:
        g_adv_chest_money = d->payload * CHEST_MONEY_UNIT;
        MoneyAdd(g_adv_chest_money);
        shows = CHEST_SHOWS_MONEY;
    }
    return shows;
}

void AdvChestShowItem(u_char actor)
{
    int n;
    int i;

    g_slot_cur = &g_slots[EFFECT_SLOT];
    CellsClear(g_adv_chest_item_cells, 10);
    CellsClear(g_adv_chest_count_cells, 2);
    n = FormatDecimal(g_adv_chest_count, g_hud_digits, 2);
    i = 0;
    CellsWriteDigitsRev(&g_adv_chest_count_cells[1], g_hud_digits, n);
    CellsWriteRow(g_adv_chest_item_cells,
                  g_item_defs[g_adv_chest_item].name, 0, 10);
    SlotInitTagged(&g_adv_chest_item_def, EFFECT_SLOT,
                   g_adv_actors[actor].z - 1,
                   g_adv_actors[actor].world_x - g_cam_x - 0x34,
                   g_adv_actors[actor].world_y - g_cam_y - 0x50);
    g_slot_cur->mx = 4;
    do {
        i++;
        AdvRunFrame();
    } while (i < HOLD_FRAMES);

    i = 0;
    do {
        g_slot_cur = &g_slots[EFFECT_SLOT];
        g_slots[EFFECT_SLOT].scale_x -= SHRINK_STEP;
        i++;
        AdvRunFrame();
    } while (i < SPIN_FRAMES);
    SlotClear(EFFECT_SLOT);
}

void AdvChestShowMoney(u_char actor)
{
    int n;
    int i;

    g_slot_cur = &g_slots[EFFECT_SLOT];
    CellsClear(g_adv_chest_money_cells, 7);
    n = FormatDecimal(g_adv_chest_money, g_hud_digits, 7);
    CellsWriteDigitsRev(&g_adv_chest_money_cells[n - 1], g_hud_digits, n);
    i = 0;
    SlotInitTagged(&g_adv_chest_money_def, EFFECT_SLOT,
                   g_adv_actors[actor].z - 1,
                   g_adv_actors[actor].world_x - g_cam_x - (n + 1) * 4,
                   g_adv_actors[actor].world_y - g_cam_y - 0x50);
    g_slot_cur->mx = 4;
    do {
        i++;
        AdvRunFrame();
    } while (i < HOLD_FRAMES);

    i = 0;
    do {
        g_slot_cur = &g_slots[EFFECT_SLOT];
        g_slots[EFFECT_SLOT].scale_x -= SHRINK_STEP;
        i++;
        AdvRunFrame();
    } while (i < SPIN_FRAMES);
    SlotClear(EFFECT_SLOT);
}

void AdvChestShowEmpty(u_char actor)
{
    Slot *s;
    int i;

    g_slot_cur = &g_slots[EFFECT_SLOT];
    CellsWriteRow(g_adv_chest_empty_cells, str_chest_empty, 0, 5);
    SlotInitTagged(&g_adv_chest_empty_def, EFFECT_SLOT,
                   g_adv_actors[actor].z - 1,
                   g_adv_actors[actor].world_x - g_cam_x - 0x14,
                   g_adv_actors[actor].world_y - g_cam_y - 0x50);
    s = g_slot_cur;
    s->mx = 4;
    s->my = 6;
    i = 0;
    do {
        i++;
        AdvRunFrame();
    } while (i < HOLD_FRAMES);

    i = 0;
    do {
        g_slot_cur = &g_slots[EFFECT_SLOT];
        g_slots[EFFECT_SLOT].scale_x -= SHRINK_STEP;
        g_slots[EFFECT_SLOT].scale_y -= SHRINK_STEP;
        g_slots[EFFECT_SLOT].rotate += SPIN_STEP;
        i++;
        AdvRunFrame();
    } while (i < SPIN_FRAMES);
    SlotClear(EFFECT_SLOT);
}
