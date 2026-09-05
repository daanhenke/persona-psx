/* Persona 1 (JP) - what a chest tells the player it gave them.
 *
 *   ADV @ 0x80081864, 0x80081A20, 0x80081BB0
 *
 * The search handler at 0x8007E814 walks the player onto the tile, finds the
 * actor standing there and reads its flag. A flag already set means the chest
 * has been taken, and AdvChestShowEmpty says so; otherwise the flag is set,
 * the opening animation plays, and what came out decides which of the other
 * two runs.
 *
 * All three work the same way: fill the sprite's cells with the text, put it
 * in slot 62 over the actor's head, hold it for 48 frames and then squeeze it
 * away over 16 more. The sprite is nudged left by half its width so it stays
 * centred, which for the money is four pixels per digit rather than a fixed
 * offset.
 */
#include <types.h>
#include <libgs.h>
#include <persona/adv/actor.h>
#include <persona/common/item.h>
#include <persona/common/slot.h>

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
