/* Persona 1 (JP) - slot record initialisers.
 *
 * Compiled into three overlays rather than called across the boundary:
 *   DNG @ 0x800757EC / 0x80075914 / 0x80075A40
 *   ADV @ 0x80065D60 / 0x80065E88 / 0x80065FB4
 *   S2D @ 0x80065850 / 0x80065978 / 0x80065AA4
 *
 * The flicker and animation setters sit further along in every overlay, past a
 * table that is not worked out yet, so they are a unit of their own in
 * slotanim.c. The prototypes all come from slot.h, so the order here is the
 * order the image has rather than anything the calls depend on.
 */
#include <decomp/types.h>
#include <persona/common/slot.h>

/* The 80 0x44-byte records live at 0x800DC10C, reached by hardcoded address
   rather than through a linker symbol. S2D's table sits 0x20000 higher, which
   is the only thing that differs between its build of this and the others'. */
#define g_slots ((Slot *)(0x800DC10C + WORK_BIAS))

/* Frees every slot, counting down. The count is the loop bound itself: the
   original starts at 80 and decrements before each call, so slot 79 is cleared
   first and slot 0 last. */
void SlotClearAll(void)
{
    int slot;

    slot = SLOT_COUNT;
    do {
        slot--;
        SlotClear(slot);
    } while (slot != 0);
}

/* Frees a slot: -1 into the script pair and the four trailing words. */
void SlotClear(u_char slot)
{
    g_slots[slot].frame = -1;
    g_slots[slot].script = -1;
    g_slots[slot].unk34 = -1;
    g_slots[slot].unk38 = -1;
    g_slots[slot].unk3C = -1;
    g_slots[slot].unk40 = -1;
}

/* Starts a slot on animation script `def` at (x, y): identity scale (0x1000 =
   1.0), full brightness (0x80), every offset cleared, marked active. `attr`
   keeps only its low 12 bits, which are the sort depth.
 *
 * The trailing scale/brightness/active stores have to stay at the end of the
 * function; putting them back in field order breaks the match.
 */
void SlotInit(void *def, u_char slot, int attr, short x, short y)
{
    g_slots[slot].script = (int)def;
    g_slots[slot].delay = 0;
    g_slots[slot].attr = attr & 0xFFF;
    g_slots[slot].x = x;
    g_slots[slot].unk18 = 0;
    g_slots[slot].unk1A = 0;
    g_slots[slot].unk1C = 0;
    g_slots[slot].unk1E = 0;
    g_slots[slot].tpage_add = 0;
    g_slots[slot].u_add = 0;
    g_slots[slot].v_add = 0;
    g_slots[slot].clut_x = 0;
    g_slots[slot].clut_y = 0;
    g_slots[slot].rotate = 0;
    g_slots[slot].mx = 0;
    g_slots[slot].my = 0;
    g_slots[slot].scale_x = 0x1000;
    g_slots[slot].scale_y = 0x1000;
    g_slots[slot].brightness = 0x80;
    g_slots[slot].active = 1;
    g_slots[slot].y = y;
}

/* Same record, but tags the slot with -2 and puts the definition pointer one
   word later - so +0x00 is a discriminator, not always a pointer. */
void SlotInitTagged(void *def, u_char slot, int attr, short x, short y)
{
    g_slots[slot].script = -2;
    g_slots[slot].frame = (int)def;
    g_slots[slot].attr = attr & 0xFFF;
    g_slots[slot].x = x;
    g_slots[slot].unk18 = 0;
    g_slots[slot].unk1A = 0;
    g_slots[slot].unk1C = 0;
    g_slots[slot].unk1E = 0;
    g_slots[slot].tpage_add = 0;
    g_slots[slot].u_add = 0;
    g_slots[slot].v_add = 0;
    g_slots[slot].clut_x = 0;
    g_slots[slot].clut_y = 0;
    g_slots[slot].rotate = 0;
    g_slots[slot].mx = 0;
    g_slots[slot].my = 0;
    g_slots[slot].scale_x = 0x1000;
    g_slots[slot].scale_y = 0x1000;
    g_slots[slot].brightness = 0x80;
    g_slots[slot].active = 1;
    g_slots[slot].y = y;
}

/* Moves a slot and rewrites the low 10 bits of its attr word, keeping bits
   10..27 and dropping the top nibble. */
void SlotSetPos(u_char slot, int attr, short x, short y)
{
    g_slots[slot].x = x;
    g_slots[slot].y = y;
    g_slots[slot].attr = (g_slots[slot].attr & 0x0FFFFC00) + attr;
}
