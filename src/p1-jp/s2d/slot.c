/* Persona 1 (JP) - slot record initialisers, S2D's copy.
 *
 * The same three as src/p1-jp/common/slot.c (which covers DNG and ADV); S2D's
 * table lives at a different address, which is the only difference.
 *   S2D @ 0x80065850 / 0x80065978 / 0x80065AA4
 */
#include <types.h>
#include <persona/common/slot.h>

/* The 0x44-byte records live at 0x800FC10C. A literal address, not a linker
   symbol: the indexed store assembles to `addu $at,rX,$at`, which is the form
   the assembler uses for a numeric base. */
#define g_slots ((Slot *)0x800FC10C)

/* Fills in one slot: identity scale, cleared everything else, marked active.
 *
 * The three constant stores sit at the end on purpose. gcc hoists them up to
 * where the original has them (right after the y argument is loaded); writing
 * them in their apparent position instead pulls them above the script store
 * and drags `active` along with them. Do not "tidy" this back into field
 * order - it drops the match to 70%.
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

/* Sets the seven animation fields in one go. The slot index arrives as a short
   here, not a u_char as everywhere else in this family. */
void SlotSetAnim(short slot, short unk18, short unk1A, u_char tpage_add,
                 u_char u_add, u_char v_add, u_short clut_x, u_short clut_y)
{
    g_slots[slot].unk18 = unk18;
    g_slots[slot].unk1A = unk1A;
    g_slots[slot].tpage_add = tpage_add;
    g_slots[slot].clut_x = clut_x;
    g_slots[slot].clut_y = clut_y;
    g_slots[slot].u_add = u_add;
    g_slots[slot].v_add = v_add;
}

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

/* Turns the flicker bit on or off. While it is set the renderer ignores the
   slot's own brightness and takes it from a 32-entry table indexed by the
   phase counter at +0x31, which it advances every frame.
 *
 * The pointer local is load-bearing twice over: it gets the scaled index
 * computed once ahead of the branch instead of rebuilt in all three basic
 * blocks, and writing the update as a compound assignment in both arms lets
 * gcc's cross-jumping merge the two stores into the shared tail the original
 * has. Spelling this as `k = ...; g_slots[i].attr = k;` costs 60%.
 */
void SlotSetFlicker(u_char slot, u_char on)
{
    Slot *s;

    s = g_slots + slot;
    if (on) {
        s->attr |= SLOT_ATTR_FLICKER;
    } else {
        s->attr &= ~SLOT_ATTR_FLICKER;
    }
}
