/* Persona 1 (JP) - the script opcode that grows a slot in.
 *
 *   ADV @ 0x800AF458
 *
 * Event script opcode 0x67, and the opposite of the 0x68 beside it: both work
 * on slot 45 about the same (0x2C, 0x30) centre. The operand picks one of
 * three x positions across the screen; the slot starts squashed at 0x40 by
 * 0x70 and gains 0x200 on both scales a frame until the horizontal one is
 * full - fourteen frames from nothing to full size. A frame is drawn between
 * steps, so the call blocks for the whole animation.
 */
#include <types.h>
#include <persona/common/slot.h>

#define g_slots ((Slot *)0x800DC10C)

#define GROW_SLOT  45
#define GROW_ATTR  0x18
#define GROW_Y     0x68
#define GROW_MX    0x2C
#define GROW_MY    0x30
#define GROW_FROM_X 0x40
#define GROW_FROM_Y 0x70
#define FULL_SCALE 0x1000
#define GROW_STEP  0x200

extern void *g_grow_slot_def;
extern short g_grow_slot_x[];

extern void AdvRunFrame(void);
extern void SlotInitTagged(void *def, u_char slot, int attr, short x, short y);

void AdvGrowSlot(u_char place)
{
    Slot *s;

    s = &g_slots[GROW_SLOT];
    SlotInitTagged(&g_grow_slot_def, GROW_SLOT, GROW_ATTR, g_grow_slot_x[place],
                   GROW_Y);
    s->scale_x = GROW_FROM_X;
    s->scale_y = GROW_FROM_Y;
    s->mx = GROW_MX;
    s->my = GROW_MY;
    do {
        AdvRunFrame();
        s->scale_x += GROW_STEP;
        if (s->scale_x > FULL_SCALE) {
            s->scale_x = FULL_SCALE;
        }
        s->scale_y += GROW_STEP;
        if (s->scale_y > FULL_SCALE) {
            s->scale_y = FULL_SCALE;
        }
    } while (s->scale_x < FULL_SCALE);
    AdvRunFrame();
}
